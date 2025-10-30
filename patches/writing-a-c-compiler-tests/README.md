# Submodule Patches

This directory contains patches for modifications made to the `writing-a-c-compiler-tests` submodule.

## Workflow

### Making Changes to the Submodule

1. Make your changes in `third-party/writing-a-c-compiler-tests/`
2. Save the changes as a patch:
   ```bash
   ./scripts/save-cli-tests-patch.sh
   ```
3. Commit the patch file to the fort repository:
   ```bash
   git add patches/writing-a-c-compiler-tests/*.patch
   git commit -m "Add patch for submodule changes"
   ```

### Applying Patches After Fresh Clone

After cloning the repository:
```bash
git submodule update --init --recursive
./scripts/apply-cli-tests-patch.sh
```

### CI/CD Integration

The CI workflow applies patches before running tests:
```yaml
- name: Checkout with submodules
  uses: actions/checkout@v4
  with:
    submodules: recursive

- name: Apply submodule patches
  run: ./scripts/apply-cli-tests-patch.sh
```

## Notes

- Patches are applied in numerical order (0001, 0002, etc.)
- Patch numbers are tracked in `NEXT_PATCH_NUM` file
- Do not commit changes directly in the submodule - always use patches
