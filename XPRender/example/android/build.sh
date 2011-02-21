ndk-build NDK_MODULE_PATH=../..

if [ ! -d assets ]; then
	mkdir assets
fi
cp ../Android.glsl assets
cp ../../media/monkey.obj assets
