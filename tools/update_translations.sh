#!/usr/bin/env bash
#
# Quét tr() trong src/ vào file .ts rồi biên dịch ra .qm.
# File .qm được commit nên bản build không cần lrelease.
#
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TS="${REPO_ROOT}/resources/i18n/sensornode_en.ts"

lupdate -no-obsolete -locations none -source-language vi -target-language en "${REPO_ROOT}/src" -ts "$TS"
lrelease "$TS" -qm "${TS%.ts}.qm"
