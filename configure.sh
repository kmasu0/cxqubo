#!/bin/bash

CURRENT_DIR=$(pwd -P)
PROJECT_DIR=$(cd $(dirname $BASH_SOURCE); pwd -P)

# BUILD_TYPE=Debug
BUILD_TYPE=${BUILD_TYPE:-Release}
BUILD_DIR=${BUILD_DIR:-$CURRENT_DIR/build-$BUILD_TYPE}
INSTALL_DIR=${INSTALL_DIR:-$CURRENT_DIR/cxqubo-$BUILD_TYPE}

rm -rf $BUILD_DIR

cmake -S$PROJECT_DIR -B$BUILD_DIR -GNinja \
  -DBUILD_SHARED_LIBS=ON       \
  -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
  -DCXQUBO_BUILD_EXAMPLE=ON

[ $? -ne 0 ] && exit 1

ln -sf $BUILD_DIR/compile_commands.json ./

SCRIPT="build-and-install.sh"
cat > $SCRIPT << EOM
#!/bin/bash

BUILD_DIR=$BUILD_DIR
cmake --build $BUILD_DIR
[ \$? -ne 0 ] && exit 1
ctest --output-on-failure --test-dir $BUILD_DIR
[ \$? -ne 0 ] && exit 1
cmake --install $BUILD_DIR
[ \$? -eq 0 ] && echo "'$INSTALL_DIR' is installed."
EOM
chmod 755 $SCRIPT

SCRIPT="unittests.sh"
cat > $SCRIPT << EOM
#!/bin/bash -xef

cd $BUILD_DIR/unittests
ctest --output-on-failure
EOM
chmod 755 $SCRIPT

