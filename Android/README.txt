
here's the general process for building and installing an Android application which uses the NDK.

1. cd to the root of its source code.

2. Run ndk-build. This builds the native code, and should result in some .so files being put into the libs directory.

3. android update project --path . --name something

4. ant debug (or similar). This will build the Java code and create an .apk. Crucially, the build process will pick up the .so files left within the libs directory and include them into the .apk.

5. adb install bin/name-of-project.apk

6. Then launch as normal using the Android GUI or using an am start command such as you give.
6a. emulator -avd Intel_x86 

7. adb shell am start -n com.example.native_activity 
     - or -
   adb shell am start com.example.native_activity/android.app.NativeActivity

8. adb uninstall com.example.native_activity


 - Matt

