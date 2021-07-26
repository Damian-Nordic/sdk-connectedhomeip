#!/usr/bin/env bash

#
#    Copyright (c) 2020 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

help() {
    echo "Usage: $0 [--sdk ANDROID-SDK-HOME] [--ndk ANDROID-NDK-HOME] [--cpu TARGET-CPU] [--build-apk]" >&2
    echo "  ANDROID-SDK-HOME path to Android SDK (defaults to \$ANDROID_HOME)" >&2
    echo "  ANDROID-NDK-HOME path to Android NDK (defaults to \$ANDROID_NDK_HOME)" >&2
    echo "  TARGET_CPU       target CPU, either arm, arm64 or x64 (defaults to \$TARGET_CPU)" >&2
    exit 1
}

declare -i BUILD_APK

while (($#)); do
    case "$1" in
        --sdk)
            ANDROID_HOME="$2"
            shift
            ;;
        --ndk)
            ANDROID_NDK_HOME="$2"
            shift
            ;;
        --cpu)
            TARGET_CPU="$2"
            shift
            ;;
        --build-apk)
            BUILD_APK=1
            ;;
        --help | -h)
            help
            ;;
        *)
            echo -e "Unknown option $1\n" >&2
            help
            ;;
    esac
    shift
done

[[ -n "$ANDROID_HOME" ]] || {
    echo -e "ANDROID_HOME is not set\n" >&2
    help
}

[[ -n "$ANDROID_NDK_HOME" ]] || {
    echo -e "ANDROID_NDK_HOME is not set\n" >&2
    help
}

[[ -n "$TARGET_CPU" ]] || {
    echo -e "TARGET_CPU is not set\n" >&2
    help
}

set -e
set -x
env

# Build shared CHIP libs
BUILD_DIR="out/android_$TARGET_CPU"
source scripts/activate.sh
gn gen --check --fail-on-unused-args "$BUILD_DIR" --args="target_os=\"android\" target_cpu=\"$TARGET_CPU\" android_ndk_root=\"$ANDROID_NDK_HOME\" android_sdk_root=\"$ANDROID_HOME\""
ninja -C "$BUILD_DIR" src/setup_payload/java src/controller/java default

rsync -a "$BUILD_DIR"/lib/*.jar src/android/CHIPTool/app/libs
rsync -a "$BUILD_DIR"/lib/jni/* src/android/CHIPTool/app/src/main/jniLibs

# Build CHIPTook APK if requested
if ((BUILD_APK)); then
    yes | "$ANDROID_HOME"/tools/bin/sdkmanager --licenses
    (cd src/android/CHIPTool && ./gradlew build)
fi
