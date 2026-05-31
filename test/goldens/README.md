# Golden References

Golden `.ppm` files under this directory are the committed render references for
`roo_windows` tests.

## Normal Test Run

Run targeted golden tests from `lib/roo_windows`:

```sh
cd lib/roo_windows
bazel test :your_golden_test --test_output=errors
```

That is the normal gating path. If the test fails, Bazel writes the rendered
`*_actual.ppm` artifacts under `bazel-testlogs/<target>/test.outputs/`.

## Update Goldens Directly

When you intentionally want to create or refresh committed goldens, do not rely
on `bazel test` to write into the workspace. The test runs in a sandbox, so the
shared helper updates goldens correctly only when you run the built test binary
directly with the workspace root exported.

From `lib/roo_windows`:

```sh
cd lib/roo_windows
bazel build :your_golden_test
BUILD_WORKSPACE_DIRECTORY=$PWD ROO_UPDATE_GOLDENS=1 ./bazel-bin/your_golden_test
```

After updating the files under `test/goldens/`, rerun the test normally:

```sh
cd lib/roo_windows
bazel test :your_golden_test --cache_test_results=no --test_output=errors
```

## Promote Existing Actuals

If a normal `bazel test` run already produced mismatch artifacts and you want to
promote those exact rendered outputs into existing golden files, use the helper
script in this directory:

```sh
cd lib/roo_windows
test/goldens/promote_golden_actuals.sh --test your_golden_test
```

This promotion flow only works for goldens that already exist and can be mapped
from the `*_actual.ppm` artifact names.