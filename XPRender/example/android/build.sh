ndk-build NDK_MODULE_PATH=../..

if [ ! -d assets ]; then
    mkdir assets
fi
cp ../Mesh.glsl assets
cp ../../media/monkey.obj assets
