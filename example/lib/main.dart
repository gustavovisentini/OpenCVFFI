import 'dart:io';
import 'dart:isolate';

import 'package:flutter/material.dart';
import 'dart:async';

import 'package:flutter/services.dart';
import 'package:image_picker/image_picker.dart';
import 'package:opencv_ffi/opencv_ffi.dart';
import 'package:path_provider/path_provider.dart';
import 'package:opencv_ffi_example/loadffi.dart';

void main() {
  WidgetsFlutterBinding.ensureInitialized();

  runApp(MyApp());
}

class MyApp extends StatefulWidget {
  @override
  _MyAppState createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  String _platformVersion = 'Unknown';

  @override
  void initState() {
    super.initState();
    initPlatformState();
  }

  // Platform messages are asynchronous, so we initialize in an async method.
  Future<void> initPlatformState() async {
    String platformVersion;
    // Platform messages may fail, so we use a try/catch PlatformException.
    // We also handle the message potentially returning null.
    try {
      platformVersion =
          await OpencvFfi.platformVersion ?? 'Unknown platform version';
    } on PlatformException {
      platformVersion = 'Failed to get platform version.';
    }

    // If the widget was removed from the tree while the asynchronous platform
    // message was in flight, we want to discard the reply rather than calling
    // setState to update our non-existent appearance.
    if (!mounted) return;

    setState(() {
      _platformVersion = platformVersion;
    });
  }

  bool _isProcessed = false;
  bool _isWorking = false;
  Image? _image = null;

  Future<void> takeImageAndProcess() async {
    final picker = new ImagePicker();
    // final image =
    //     await picker.pickImage(source: ImageSource.gallery, imageQuality: 100);
    var image = await picker.getImage(source: ImageSource.gallery);

    print(image?.path);

    if (image == null) {
      return;
    }

    setState(() {
      _isWorking = true;
    });

    Directory tempDir = await getTemporaryDirectory();
    String tempPath = tempDir.path + '/output.png';

    // Creating a port for communication with isolate and arguments for entry point
    final port = ReceivePort();
    final args = ProcessImageArguments(image.path, tempPath);

    // Spawning an isolate
    Isolate.spawn<ProcessImageArguments>(processImage, args,
        onError: port.sendPort, onExit: port.sendPort);

    // Making a variable to store a subscription in
    StreamSubscription? sub;

    // Listeting for messages on port
    sub = port.listen((_) async {
      // Cancel a subscription after message received called
      await sub?.cancel();

      setState(() {
        _isProcessed = true;
        _isWorking = false;

        print(tempPath);
        File processed = File(tempPath);

        final bytes = processed.readAsBytesSync();

        _image = Image.memory(bytes);

        print(_image);
      });
    });
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Plugin example app'),
        ),
        body: Center(
            child: Column(
          children: [
            TextButton(
              onPressed: () {
                takeImageAndProcess();
              },
              child: Text('Processar'),
            ),
            _image != null ? _image! : Text(''),
            Text(
                'Running on: $_platformVersion\nOpenCV Version: ${opencvVersion()}'),
          ],
        )),
      ),
    );
  }
}
