#!/usr/bin/env bash
# =============================================================================
# TweetNaCl-Modular — checkpatch.pl Lint Script
# =============================================================================
# Runs Linux kernel's checkpatch.pl against C source files with project-specific
# configuration. Designed for C libraries (not kernel modules), so many
# kernel-specific warnings are suppressed.
#
# Usage:
#   ./scripts/checkpatch.sh              # Lint all C files
#   ./scripts/checkpatch.sh src/foo.c    # Lint a specific file
#   ./scripts/checkpatch.sh --fix       # Show fixable patches
# =============================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
CHECKPATCH_SCRIPT="$REPO_ROOT/scripts/checkpatch.pl"
CONFIG_FILE="$REPO_ROOT/.checkpatchrc"

# ── Colors ───────────────────────────────────────────────────────────────────
RED='\033[0;31m'
YELLOW='\033[1;33m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# ── Defaults ─────────────────────────────────────────────────────────────────
VERBOSE=0
FIX_MODE=0
MAX_LINE_LENGTH=100
ERRORS_AS_WARNINGS=0  # Set to 1 to treat errors as warnings (CI default)

# ── Parse arguments ─────────────────────────────────────────────────────────
FILES=()
while [[ $# -gt 0 ]]; do
    case "$1" in
        --fix)
            FIX_MODE=1
            shift
            ;;
        --verbose|-v)
            VERBOSE=1
            shift
            ;;
        --strict)
            ERRORS_AS_WARNINGS=0
            shift
            ;;
        --len=*)
            MAX_LINE_LENGTH="${1#--len=}"
            shift
            ;;
        --)
            shift
            FILES+=("$@")
            break
            ;;
        -*)
            echo "Unknown option: $1"
            exit 1
            ;;
        *)
            FILES+=("$1")
            shift
            ;;
    esac
done

# ── Load config overrides ───────────────────────────────────────────────────
if [[ -f "$CONFIG_FILE" ]]; then
    # shellcheck source=/dev/null
    source "$CONFIG_FILE"
fi

# ── Discover files ───────────────────────────────────────────────────────────
if [[ ${#FILES[@]} -eq 0 ]]; then
    mapfile -t FILES < <(
        find "$REPO_ROOT/src" \
             "$REPO_ROOT/include" \
             "$REPO_ROOT/tests" \
             -name '*.c' -o -name '*.h' |
        grep -vE '(vendor|third_party|generated)' |
        sort
    )
fi

if [[ ${#FILES[@]} -eq 0 ]]; then
    echo "No C files found to check."
    exit 0
fi

# ── Check for checkpatch.pl ─────────────────────────────────────────────────
if [[ ! -f "$CHECKPATCH_SCRIPT" ]]; then
    echo -e "${YELLOW}checkpatch.pl not found. Downloading from kernel source...${NC}"
    echo "  Option 1: git clone --depth=1 https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git"
    echo "  Option 2: curl -o scripts/checkpatch.pl https://raw.githubusercontent.com/torvalds/linux/master/scripts/checkpatch.pl"
    echo ""
    echo "  Then run: $0"
    exit 1
fi

# ── Build checkpatch arguments ───────────────────────────────────────────────
CHECKPATCH_ARGS=(
    --max-line-length="$MAX_LINE_LENGTH"
    --no-tree
    --no-summary
    --no-signoff
    --no-commit
    --terse
    # ── Suppress kernel-specific warnings (not relevant for C libraries) ──
    --types=
    --ignore=
    # ── Allow patterns specific to this project ──
    --allow=
)

# Load suppressions from config
if [[ -f "$CONFIG_FILE" ]]; then
    # Suppress types (warnings to ignore)
    if [[ -n "${CHECKPATCH_SUPPRESS_TYPES:-}" ]]; then
        CHECKPATCH_ARGS+=(--types="$CHECKPATCH_SUPPRESS_TYPES")
    fi
    # Ignore patterns (regex patterns to ignore)
    if [[ -n "${CHECKPATCH_IGNORE_PATTERNS:-}" ]]; then
        CHECKPATCH_ARGS+=(--ignore="$CHECKPATCH_IGNORE_PATTERNS")
    fi
    # Allow patterns (patterns to explicitly allow)
    if [[ -n "${CHECKPATCH_ALLOW_PATTERNS:-}" ]]; then
        CHECKPATCH_ARGS+=(--allow="$CHECKPATCH_ALLOW_PATTERNS")
    fi
fi

# ── Run checkpatch ───────────────────────────────────────────────────────────
TOTAL_FILES=${#FILES[@]}
ERROR_COUNT=0
WARNING_COUNT=0
FIX_COUNT=0

echo "Running checkpatch on $TOTAL_FILES file(s)..."
echo "  Max line length: $MAX_LINE_LENGTH"
echo "  Config: $CONFIG_FILE"
echo "  checkpatch.pl: $CHECKPATCH_SCRIPT"
echo ""

for FILE in "${FILES[@]}"; do
    REL_PATH="${FILE#$REPO_ROOT/}"

    if [[ $FIX_MODE -eq 1 ]]; then
        # Show fixable patches
        PATCH=$(bash "$CHECKPATCH_SCRIPT" "${CHECKPATCH_ARGS[@]}" --fix "$FILE" 2>&1) || true
        if [[ -n "$PATCH" ]]; then
            echo -e "${YELLOW}  [FIX] $REL_PATH${NC}"
            echo "$PATCH" | head -20
            echo "  ..."
            FIX_COUNT=$((FIX_COUNT + 1))
        fi
    else
        OUTPUT=$(bash "$CHECKPATCH_SCRIPT" "${CHECKPATCH_ARGS[@]}" "$FILE" 2>&1) || true
        if [[ -n "$OUTPUT" ]]; then
            # Count errors and warnings
            FILE_ERRORS=$(echo "$OUTPUT" | grep -c "ERROR:" || true)
            FILE_WARNINGS=$(echo "$OUTPUT" | grep -c "WARNING:" || true)

            if [[ $FILE_ERRORS -gt 0 ]]; then
                echo -e "${RED}  [ERROR] $REL_PATH ($FILE_ERRORS errors)${NC}"
                if [[ $VERBOSE -eq 1 ]]; then
                    echo "$OUTPUT" | grep "ERROR:"
                fi
                ERROR_COUNT=$((ERROR_COUNT + FILE_ERRORS))
            fi

            if [[ $FILE_WARNINGS -gt 0 ]]; then
                echo -e "${YELLOW}  [WARN]  $REL_PATH ($FILE_WARNINGS warnings)${NC}"
                if [[ $VERBOSE -eq 1 ]]; then
                    echo "$OUTPUT" | grep "WARNING:"
                fi
                WARNING_COUNT=$((WARNING_COUNT + FILE_WARNINGS))
            fi
        fi
    fi
done

# ── Summary ──────────────────────────────────────────────────────────────────
echo ""
echo "═══════════════════════════════════════════════════════════"
if [[ $FIX_MODE -eq 1 ]]; then
    echo -e "  ${YELLOW}Fixable issues found in $FIX_COUNT file(s)${NC}"
else
    echo -e "  ${RED}Errors:   $ERROR_COUNT${NC}"
    echo -e "  ${YELLOW}Warnings: $WARNING_COUNT${NC}"
    echo -e "  ${GREEN}Passed:   $((TOTAL_FILES - ERROR_COUNT))/$TOTAL_FILES files${NC}"
fi
echo "═══════════════════════════════════════════════════════════"

# Exit with error if there are errors (unless in CI lenient mode)
if [[ $ERRORS_AS_WARNINGS -eq 1 && $ERROR_COUNT -gt 0 ]]; then
    echo -e "${YELLOW}CI mode: treating errors as warnings${NC}"
    exit 0
elif [[ $ERROR_COUNT -gt 0 ]]; then
    exit 1
fi

exit 0
