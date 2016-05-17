package com.noregular.penman.app;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.Toast;

import com.noregular.penman.R;

/**
 * 
 * The activity that searches for the textfield and is able to capture it.
 * 
 * @author Lucas Swartsenburg (6174388)
 * 
 */
public class FindFieldActivity extends Activity {
	private static final String TAG = "FindFieldActivity";

	private FindFieldView mView;

	/**
	 * Closes the camera.
	 */
	@Override
	protected void onPause() {
		Log.i(TAG, "onPause");
		super.onPause();
		mView.releaseCamera();
		finish();
	}

	/**
	 * Cleans up the view.
	 */
	@Override
	public void onDestroy() {
		super.onDestroy();
		
	}

	/**
	 * Opens the camera when the app returns.
	 */
	@Override
	protected void onResume() {
		Log.i(TAG, "onResume");
		super.onResume();
		if (!mView.openCamera()) {
			AlertDialog ad = new AlertDialog.Builder(this).create();
			ad.setCancelable(false); // This blocks the 'BACK' button
			ad.setMessage("Fatal error: can't open camera!");
			ad.setButton("OK", new DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int which) {
					dialog.dismiss();
					finish();
				}
			});
			ad.show();
		}
	}

	/**
	 * Sets all window parameters.
	 */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState); 
		Log.i(TAG, "onCreate");

		Window window = getWindow();
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		window.setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
				WindowManager.LayoutParams.FLAG_FULLSCREEN);
		window.addFlags(WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED
				| WindowManager.LayoutParams.FLAG_DISMISS_KEYGUARD);
		window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON
				| WindowManager.LayoutParams.FLAG_TURN_SCREEN_ON);

		setContentView(R.layout.video_main);

		Intent i = getIntent();
		String msg = i.getStringExtra("msg");
		if(msg!=null && !msg.equals("")){
			Toast.makeText(FindFieldActivity.this, msg, Toast.LENGTH_LONG).show(); 
		}
		
		LinearLayout main = (LinearLayout) findViewById(R.id.findfield);
		Button but = (Button) findViewById(R.id.button1);
		mView = new FindFieldView(FindFieldActivity.this, but, this);
		main.addView(mView);

	}

}
