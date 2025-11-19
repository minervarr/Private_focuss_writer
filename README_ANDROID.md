# Phantom Writer - Android Build Instructions

This document provides detailed instructions for building and running Phantom Writer on Android.

## Prerequisites

### Required Software

1. **Android Studio** (Arctic Fox or newer)
   - Download from: https://developer.android.com/studio

2. **Android SDK**
   - API Level 26 or higher (Android 8.0+)
   - Install via Android Studio SDK Manager

3. **Android NDK**
   - Version 25.2.9519653 or higher
   - Install via Android Studio SDK Manager

4. **CMake**
   - Version 3.22.1 or higher
   - Install via Android Studio SDK Manager

5. **Vulkan SDK** (for development/debugging)
   - Download from: https://vulkan.lunarg.com/

### Device Requirements

- **Android Version**: 8.0 (API 26) or higher
- **Vulkan Support**: Required (hardware feature)
- **Architecture**: ARM64-v8a or ARMv7

## Quick Start

### Option 1: Using Android Studio (Recommended)

1. **Open Project in Android Studio**
   ```bash
   # Navigate to the android directory
   cd Private_focuss_writer/android

   # Open with Android Studio
   studio .
   # Or: File -> Open -> Select 'android' directory
   ```

2. **Sync Gradle**
   - Android Studio will automatically prompt to sync Gradle
   - Or manually: File -> Sync Project with Gradle Files

3. **Build Project**
   - Build -> Make Project (Ctrl+F9 / Cmd+F9)

4. **Run on Device/Emulator**
   - Connect Android device via USB with USB debugging enabled
   - Or create an Android Emulator (must support Vulkan)
   - Run -> Run 'app' (Shift+F10 / Ctrl+R)

### Option 2: Command Line Build

1. **Set up environment**
   ```bash
   export ANDROID_HOME=/path/to/Android/Sdk
   export ANDROID_NDK_HOME=$ANDROID_HOME/ndk/25.2.9519653
   ```

2. **Build APK**
   ```bash
   cd android
   ./gradlew assembleDebug
   ```

3. **Install APK**
   ```bash
   # Find the built APK
   # app/build/outputs/apk/debug/app-debug.apk

   # Install to connected device
   adb install app/build/outputs/apk/debug/app-debug.apk
   ```

4. **Launch App**
   ```bash
   adb shell am start -n com.phantom.writer/.MainActivity
   ```

## Build Configuration

### Build Types

**Debug Build:**
```bash
./gradlew assembleDebug
```
- Includes debugging symbols
- No optimization
- Logging enabled

**Release Build:**
```bash
./gradlew assembleRelease
```
- Optimized for performance
- Smaller APK size
- Requires signing for distribution

### Supported Architectures

By default, the app builds for:
- `arm64-v8a` (64-bit ARM - recommended)
- `armeabi-v7a` (32-bit ARM)

To build for specific ABI:
```gradle
// In app/build.gradle
ndk {
    abiFilters 'arm64-v8a'  // Only 64-bit
}
```

## Project Structure

```
Private_focuss_writer/
├── android/
│   ├── app/
│   │   ├── src/main/
│   │   │   ├── java/com/phantom/writer/
│   │   │   │   └── MainActivity.kt      # Android activity
│   │   │   ├── cpp/                     # Native code entry
│   │   │   ├── res/                     # Android resources
│   │   │   └── AndroidManifest.xml      # App manifest
│   │   └── build.gradle                 # App build config
│   ├── build.gradle                     # Project build config
│   ├── settings.gradle                  # Project settings
│   ├── gradle.properties                # Gradle properties
│   └── CMakeLists.txt                   # Native build config
└── src/platform/android/                # Android platform implementation
    ├── window_android.h/cpp
    ├── input_android.h/cpp
    ├── file_system_android.h/cpp
    ├── platform_android.cpp
    └── android_main.cpp                 # Native entry point
```

## Development

### Opening in Android Studio

1. Open Android Studio
2. File -> Open
3. Navigate to `Private_focuss_writer/android`
4. Click "OK"

### Debugging

**Enable USB Debugging on Device:**
1. Settings -> About Phone
2. Tap "Build Number" 7 times to enable Developer Options
3. Settings -> Developer Options
4. Enable "USB Debugging"

**Debug in Android Studio:**
1. Set breakpoints in C++ code
2. Run -> Debug 'app' (Shift+F9 / Ctrl+D)
3. Use Android Studio debugger with LLDB support

### Logging

View logs using logcat:
```bash
# Filter by app tag
adb logcat -s PhantomWriter

# View all logs
adb logcat
```

In code:
```cpp
LOG_INFO(LogCategory::PLATFORM, "Your message here");
```

## Troubleshooting

### Vulkan Not Found

**Error**: "Vulkan library not found"

**Solution**:
1. Ensure device has Android 8.0+
2. Verify Vulkan support: `adb shell getprop ro.hardware.vulkan`
3. Use device with Vulkan 1.0+ support

### CMake Configuration Failed

**Error**: "CMake configuration failed"

**Solution**:
1. Check CMake version in Android Studio (3.22.1+)
2. Tools -> SDK Manager -> SDK Tools -> CMake
3. Sync Gradle: File -> Sync Project with Gradle Files

### NDK Not Found

**Error**: "NDK not found"

**Solution**:
1. Install NDK via Android Studio
2. Tools -> SDK Manager -> SDK Tools -> NDK
3. Set NDK path in `gradle.properties`:
   ```properties
   android.ndkVersion=25.2.9519653
   ```

### Build Errors in Native Code

**Error**: C++ compilation errors

**Solution**:
1. Clean build: Build -> Clean Project
2. Rebuild: Build -> Rebuild Project
3. Invalidate caches: File -> Invalidate Caches / Restart

### App Crashes on Launch

**Check Logs:**
```bash
adb logcat | grep -i phantom
```

**Common Issues:**
1. Missing Vulkan support on device
2. Missing assets (fonts, shaders)
3. Incorrect permissions in AndroidManifest.xml

### Performance Issues

**Enable GPU Profiling:**
```bash
adb shell setprop debug.vulkan.layers VK_LAYER_KHRONOS_validation
```

**Profile with Android Studio:**
1. Run -> Profile 'app'
2. Use CPU, Memory, and GPU profilers

## Testing on Emulator

**Create Vulkan-Compatible Emulator:**

1. AVD Manager -> Create Virtual Device
2. Select a device (Pixel 4+ recommended)
3. System Image: API 26+ with "Google APIs" or "Google Play"
4. Advanced Settings:
   - Graphics: Hardware - GLES 3.0
   - Multi-Core CPU: 4+ cores
   - RAM: 4GB+

**Note**: Not all emulators support Vulkan fully. Physical device testing is recommended.

## Building for Distribution

### 1. Generate Signed APK

**Create Keystore:**
```bash
keytool -genkey -v -keystore phantom-writer.keystore \
        -alias phantom_key -keyalg RSA -keysize 2048 -validity 10000
```

**Configure in app/build.gradle:**
```gradle
android {
    signingConfigs {
        release {
            storeFile file("path/to/phantom-writer.keystore")
            storePassword "your_store_password"
            keyAlias "phantom_key"
            keyPassword "your_key_password"
        }
    }
    buildTypes {
        release {
            signingConfig signingConfigs.release
            ...
        }
    }
}
```

**Build Release APK:**
```bash
./gradlew assembleRelease
```

### 2. Build App Bundle (for Google Play)

```bash
./gradlew bundleRelease
```

Output: `app/build/outputs/bundle/release/app-release.aab`

## Performance Optimization

### Shader Compilation

Shaders are pre-compiled to SPIR-V and included in APK assets.

### Native Library Optimization

- Release builds use `-O3` optimization
- Link-Time Optimization (LTO) enabled
- RTTI and exceptions disabled for size reduction

### APK Size Optimization

Enable ProGuard/R8:
```gradle
buildTypes {
    release {
        minifyEnabled true
        shrinkResources true
    }
}
```

## Getting Help

If you encounter issues:

1. Check logcat output: `adb logcat -s PhantomWriter`
2. Verify Vulkan support: `adb shell dumpsys vulkan`
3. Check device info: `adb shell getprop | grep vulkan`
4. Review Android Studio build logs

## Requirements Summary

| Requirement | Minimum | Recommended |
|-------------|---------|-------------|
| Android Version | 8.0 (API 26) | 11+ (API 30) |
| Vulkan Version | 1.0 | 1.1+ |
| RAM | 2GB | 4GB+ |
| Storage | 100MB | 200MB+ |
| Architecture | ARMv7 | ARM64 |

## License

See LICENSE file for details.
