#!/bin/bash

# Compile wheels
/opt/python/cp35-cp35m/bin/pip wheel /io/ -w wheelhouse/
/opt/python/cp36-cp36m/bin/pip wheel /io/ -w wheelhouse/
/opt/python/cp37-cp37m/bin/pip wheel /io/ -w wheelhouse/

# Bundle external shared libraries into the wheels
for whl in wheelhouse/**.whl; do
    auditwheel repair "$whl" --plat $PLAT -w /io/wheelhouse/
done
