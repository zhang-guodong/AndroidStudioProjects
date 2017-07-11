package com.example.zgd.result;

import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

import java.io.IOException;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    public static String OUTPUT_DIRECTORY =
            //"/output";
            //"/system/xbin/output";
            "/sdcard/output";
            //Environment.getExternalStorageDirectory().getAbsolutePath() + "/output";

    @Override
    protected void onCreate(Bundle savedInstanceState) {



        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = (TextView) findViewById(R.id.sample_text);
        tv.setText(getRoot());

//        super.onCreate(savedInstanceState);
//        setContentView(R.layout.activity_main);

        try {
            UnzipAssets.unZip(MainActivity.this, "res.zip", OUTPUT_DIRECTORY, true);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
    public native String getRoot();
    public native String test();
}
