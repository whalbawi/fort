#!/usr/bin/env bash
set -eux

PROJECT_ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
THIRD_PARTY_DIR="$PROJECT_ROOT"/third-party
CLI_TESTS_DIR="$THIRD_PARTY_DIR"/writing-a-c-compiler-tests

CLI_TESTS=$CLI_TESTS_DIR/test_compiler
FORT="$PROJECT_ROOT"/build/fort

CHAPTER=1
STAGE=codegen

$CLI_TESTS $FORT --chapter $CHAPTER --stage $STAGE