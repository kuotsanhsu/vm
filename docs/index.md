## Web links
- [Hypervisor.Framework Objective-C](https://developer.apple.com/documentation/hypervisor?language=objc)
- [Demonstrates Hypervisor.Framework usage in Apple Silicon](https://gist.github.com/imbushuo/51b09e61ecd7b7ac063853ad65cedf34)
- [clangd system headers](https://clangd.llvm.org/guides/system-headers)

## Commands
- Get Makefile setting: `make -p | code -`
- Find compiler search path: `cc -v -c -xc /dev/null`
  - /Library/Developer/CommandLineTools/usr/lib/clang/16/include
  - /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include
  - /Library/Developer/CommandLineTools/usr/include
  - /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Frameworks
- Hypervisor framework path: /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Frameworks/Hypervisor.framework/Headers

## Code signing

- [Creating distribution-signed code for macOS](https://developer.apple.com/documentation/xcode/creating-distribution-signed-code-for-the-mac)
- [How to add entitlements to a command line program written in C](https://stackoverflow.com/questions/65502057/how-to-add-entitlements-to-a-command-line-program-written-in-c)
- [Tips And Tricks for Shipping a PyGame App on the Mac](https://glyph.twistedmatrix.com/2018/01/shipping-pygame-mac-app.html)
  - Notarization
- [Use macOS . entitlements with SwiftPM or swift command line](https://forums.swift.org/t/use-macos-entitlements-with-swiftpm-or-swift-command-line/42230)
- [Editing Property Lists with plutil](https://scriptingosx.com/2016/11/editing-property-lists/)
- /Users/gordonh/Documents/RunningLinuxInAVirtualMachine/LinuxVirtualMachine/LinuxVirtualMachine.entitlements
    ```xml
    <?xml version="1.0" encoding="UTF-8"?>
    <!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
    <plist version="1.0">
    <dict>
      <key>com.apple.security.virtualization</key>
      <true/>
    </dict>
    </plist>
    ```

```sh
open x-man-page:///plutil
open x-man-page:///PlistBuddy
plutil -help # NOTE: there is only one dash in the argument "-help"
/usr/libexec/PlistBuddy --help

plutil -create xml1 entitlements.plist
plutil -insert "com\.apple\.security\.hypervisor" -bool true $_

# "ad-hoc signing" with the "signing identity" being "-"
make -B main # --always-make
codesign --display --entitlements - main # No entitlements
./main # [ERROR] (?/?)  (fae94007)
codesign --sign - --entitlements entitlements.plist main
codesign --display --entitlements - main # com.apple.security.hypervisor
./main # No errors
codesign --sign - --entitlements entitlements.plist main # main: is already signed
./main # No errors
codesign --sign - --entitlements entitlements.plist --force main # main: replacing existing signature
./main # No errors
```

## Nested virtualization
- [nestedVirtualizationSupported](https://developer.apple.com/documentation/virtualization/vzgenericplatformconfiguration/isnestedvirtualizationsupported?language=objc)
