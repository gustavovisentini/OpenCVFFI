import 'dart:ffi' as ffi;
import 'dart:io';
import 'package:ffi/ffi.dart';

// C function signatures
typedef VersionFunc = ffi.Pointer<Utf8> Function();
typedef ProcessImageFunc = ffi.Pointer<Utf8> Function(
    ffi.Pointer<Utf8>, ffi.Pointer<Utf8>);

// Dart function signatures
typedef _VersionFunc = ffi.Pointer<Utf8> Function();
typedef _ProcessImageFunc = ffi.Pointer<Utf8> Function(
    ffi.Pointer<Utf8>, ffi.Pointer<Utf8>);

// Getting a library that holds needed symbols
ffi.DynamicLibrary _lib = Platform.isAndroid
    ? ffi.DynamicLibrary.open('libopencv_ffi.so')
    : ffi.DynamicLibrary.process();

// Looking for the functions
final _VersionFunc _version =
    _lib.lookup<ffi.NativeFunction<VersionFunc>>('version').asFunction();
final _ProcessImageFunc _processImage = _lib
    .lookup<ffi.NativeFunction<ProcessImageFunc>>('process_image')
    .asFunction();

String opencvVersion() {
  return _version().toDartString();
}

class ProcessImageArguments {
  final String inputPath;
  final String outputPath;

  ProcessImageArguments(this.inputPath, this.outputPath);
}

String processImage(ProcessImageArguments args) {
  return _processImage(
          args.inputPath.toNativeUtf8(), args.outputPath.toNativeUtf8())
      .toDartString();
}
