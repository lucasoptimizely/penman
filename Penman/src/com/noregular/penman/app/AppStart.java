package com.noregular.penman.app;

/**
 * This class is designed and developed by neo for Zetacom. 
 * */

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.Window;
import android.view.WindowManager;

import com.noregular.penman.R;


public class AppStart extends Activity {
	public static final String LOG_TAG = "AppStart";
	
	
	

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		Log.d(LOG_TAG, "Started activity AppStart");
		super.onCreate(savedInstanceState);
		
		setUI();
		
		initStart();


	}
	
	
	/**
	 * Function that delays the switching to the next activity for 3 seconds. 
	 * 
	 * 
	 * @param method
	 */

	public void initStart(){
		Thread t = new Thread(new Runnable() {
			
			public void run() {
				try {
					Thread.sleep(3000);
				} catch (Exception e) {
					Log.e(LOG_TAG, e.getMessage(), e);
				}

				myHandler.sendEmptyMessage(0);
			}
		});

		t.start();
	}
	
	public void setUI(){
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
				WindowManager.LayoutParams.FLAG_FULLSCREEN);
		setContentView(R.layout.appstart);
	}

	



	/**
	 * Called after the timeout. 
	 */
	private Handler myHandler = new Handler() {
		@Override
		public void handleMessage(Message msg) {
			super.handleMessage(msg);

			

			Intent i = new Intent(AppStart.this, FindFieldActivity.class);
			AppStart.this.startActivity(i);			
			

			finish();
		}
	};


}