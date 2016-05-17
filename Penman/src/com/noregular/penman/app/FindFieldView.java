package com.noregular.penman.app;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.List;

import org.opencv.android.Utils;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.core.MatOfRect;
import org.opencv.features2d.KeyPoint;
import org.opencv.imgproc.Imgproc;

import android.app.Activity;
import android.app.NativeActivity;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.PictureCallback;
import android.hardware.Camera.ShutterCallback;
import android.hardware.Camera.Size;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.Button;

import com.noregular.penman.R;
import com.noregular.penman.ins.PassPicture;
import com.noregular.penman.mod.SVM;

/**
 * This class is responsible for finding the textfield and capturing it.
 * 
 * @author Lucas Swartsenburg (6174388)
 * 
 */
class FindFieldView extends FindFieldViewBase {

	private static final String TAG = "FindFieldView";

	private File SVMFile;
	private SVM NativeSVM;

	private Mat mYuv;
	private Mat mRgba;
	private Mat mGraySubmat;
	private Mat mIntermediateMat;
	private FpsMeter meter = new FpsMeter();
	private Button trigger;

	private Bitmap mBitmap;
	private Bitmap bit;
	private Context context;
	private Activity caller;

	ProgressDialog progressBar;

	private double width = 0;
	private double height = 0;
	private String svmlocation = "";
	

	
	private KeyPoint[] kps;

	public FindFieldView(Context context, Button button, final Activity caller) {
		super(context);
		trigger = button;
		this.context = context;
		this.caller = caller;
		final Context c = context;
		button.setOnClickListener(new View.OnClickListener() {
			public void onClick(View arg0) {
				progressBar = new ProgressDialog(c);
				progressBar.setCancelable(true);
				progressBar.setOnCancelListener(new DialogInterface.OnCancelListener() {
					
					public void onCancel(DialogInterface dialog) {
						Intent intent = caller.getIntent();
						caller.finish();
						caller.startActivity(intent);

						
					}
				});
				progressBar.setMessage("Warping the textfield...");
				progressBar.show();				
				takePicture();

			}
		});

		try {
			InputStream is = context.getResources().openRawResource(
					R.raw.svmsurf);
			File cascadeDir = context.getDir("cascade", Context.MODE_PRIVATE);
			SVMFile = new File(cascadeDir, "lbpcascade_frontalface.xml");
			FileOutputStream os = new FileOutputStream(SVMFile);

			byte[] buffer = new byte[4096];
			int bytesRead;
			while ((bytesRead = is.read(buffer)) != -1) {
				os.write(buffer, 0, bytesRead);
			}
			is.close();
			os.close();
			
			NativeSVM = new SVM(SVMFile.getAbsolutePath());
			
			
			svmlocation = SVMFile.getAbsolutePath();

		} catch (IOException e) {
			e.printStackTrace();
			Log.e(TAG, "Failed to load cascade. Exception thrown: " + e);
		}
	}

	// source: OpenCV Tutorial 4 (Advanced) - Mix Java + Native OpenCV
	@Override
	protected void onPreviewStarted(int previewWidtd, int previewHeight) {
		// initialize Mats before usage
		mYuv = new Mat(getFrameHeight() + getFrameHeight() / 2,
				getFrameWidth(), CvType.CV_8UC1);
		mGraySubmat = mYuv.submat(0, getFrameHeight(), 0, getFrameWidth());

		mRgba = new Mat();
		mIntermediateMat = new Mat();

		mBitmap = Bitmap.createBitmap(getFrameWidth(), getFrameHeight(),
				Bitmap.Config.ARGB_8888);
	}

	// source: OpenCV Tutorial 4 (Advanced) - Mix Java + Native OpenCV
	@Override
	protected void onPreviewStopped() {

		if (mBitmap != null) {
			mBitmap.recycle();
			mBitmap = null;
		}

		// Explicitly deallocate Mats
		if (mYuv != null)
			mYuv.release();
		if (mRgba != null)
			mRgba.release();
		if (mGraySubmat != null)
			mGraySubmat.release();
		if (mIntermediateMat != null)
			mIntermediateMat.release();

		mYuv = null;
		mRgba = null;
		mGraySubmat = null;
		mIntermediateMat = null;

	}

	@Override
	protected Bitmap processFrame(byte[] data) {
		mYuv.put(0, 0, data);
		
		Imgproc.cvtColor(mYuv, mRgba, Imgproc.COLOR_YUV420sp2RGB, 4);
		bit = mBitmap;
		Utils.matToBitmap(mRgba, bit);
		meter.measure();
		Mat out = new Mat(14, 1, CvType.CV_64FC1);
		kps = new KeyPoint[4];

		if (NativeSVM.detect(mGraySubmat, mRgba, out)) {

			setParams(out);

			caller.runOnUiThread(new Runnable() {
				public void run() {
					trigger.setVisibility(VISIBLE);

				}
			});
		} else {
			caller.runOnUiThread(new Runnable() {
				public void run() {
					trigger.setVisibility(INVISIBLE);
				}
			});

		}

		
		Bitmap bmp = mBitmap;

		// source: OpenCV Tutorial 4 (Advanced) - Mix Java + Native OpenCV
		try {
			Utils.matToBitmap(mRgba, bmp);
			Canvas c = new Canvas(bmp);
			meter.draw(c, 10, 10); 
		} catch (Exception e) {
			Log.e(TAG,
					"Utils.matToBitmap() throws an exception: "
							+ e.getMessage());
			bmp.recycle();
			bmp = null;
		}

		return bmp;
	}

	private synchronized void setParams(Mat out) {
		double temp[];

		temp = out.get(0, 0);

		width = temp[0];
		temp = out.get(1, 0);
		height = temp[0];
		Log.v(TAG, "width: " + width + ", height: " + height);
		for (int i = 0; i < 4; i++) {
			int r = (i * 3) + 2;
			temp = out.get(r, 0);
			kps[i] = new KeyPoint();
			kps[i].size = (float) temp[0];
			temp = out.get(r + 1, 0); 
			kps[i].pt.x = (float) temp[0];
			temp = out.get(r + 2, 0); 
			kps[i].pt.y = (float) temp[0]; 
		}
	}

	public synchronized void takePicture() {
		double w = width;
		double h = height; 
		KeyPoint[] k = kps.clone();
		if (k.length != 4){
			if(progressBar!=null)
				progressBar.cancel();
			return;
		}
		PassPicture pp = PassPicture.getInstance();

		Log.v(TAG, "width: " + w + ", height: " + h);

		for (int i = 0; i < 4; i++) {
			if (k[i] == null) {
				Log.d(TAG, "not all keypoints are set");
				if(progressBar!=null)
					progressBar.cancel();				
				return;
			}
		}
		
		Size sp = mCamera.getParameters().getPreviewSize();

		
		releaseCamera();
		System.gc();
		Camera cam = Camera.open();
		
		Parameters p = cam.getParameters();
		
		Size s = getOptimalPictureSize(sp.width, sp.height, p);
		p.setRotation(0);
		
		p.setPictureSize(s.width, s.height);
		cam.setParameters(p);
		
		Log.d(TAG, "Picture size: " + s.width + "x" +s.height);

		double fx = (double)s.width / (double)w; 
		double fy = (double)s.height / (double)h;
		double av = ((fx + fy) / 2.);

		for (int i = 0; i < 4; i++) {
			if (k[i] == null) {
				Log.d(TAG, "not all keypoints are set");
				return;
			}
			Log.v(TAG, "mult: " + av + ", " + fx + ", " + fy);

			k[i].size = (float) (k[i].size * av);
			k[i].pt.x = (float) (k[i].pt.x * fx);
			k[i].pt.y = (float) (k[i].pt.y * fy);
		}		
		

		pp.setK(k);
		pp.setSVM(svmlocation);
		Log.d(TAG, "take picture");
		System.gc();
		takePic(cam, s, pp);

	}

	public synchronized void takePic(Camera cam, Size s, PassPicture pp) {
		System.gc();
		cam.takePicture(shutterCallback, rawCallback, new afterPicture(pp,
				context, s.width, s.height));
	}	
	
	public void destroy() {
		NativeSVM.release();
	}

	
	private Size getOptimalPictureSize(int w, int h, Parameters p) {
		double targetRatio = (double) w / h;

		List<Size> sizes = p.getSupportedPictureSizes();



		Size highest = null;

		int highestheight = 0;
		// Try to find an size match aspect ratio and size
		for (Size size : sizes) {
			double ratio = (double) size.width / size.height;
			if (ratio == targetRatio)
				continue;
			
			
			if (size.height > highestheight) {
				highest = size;
				highestheight = size.height;
			}
		}


		return highest;
	}	
	
	private Size setOptimalPictureSize(int w, int h) {
		final double ASPECT_TOLERANCE = 0.1;
		double targetRatio = (double) w / h;
		Camera.Parameters params = mCamera.getParameters();
		List<Size> sizes = params.getSupportedPictureSizes();

		Size optimalSize = null;
		double minDiff = Double.MAX_VALUE;

		int targetHeight = h;

		Size highest = null;

		int highestheight = 0;
		// Try to find an size match aspect ratio and size
		for (Size size : sizes) {
			
			
			double ratio = (double) size.width / size.height;
			if (size.height > highestheight && size.height < 800) {
				highest = size;
				highestheight = size.height;
			}
			if (Math.abs(ratio - targetRatio) > ASPECT_TOLERANCE)
				continue;
			if (Math.abs(size.height - targetHeight) < minDiff
					&& size.width > w) {
				optimalSize = size;
				minDiff = Math.abs(size.height - targetHeight);
			}
		}

		// Cannot find the one match the aspect ratio, ignore the requirement
		if (optimalSize == null) {
			minDiff = Double.MAX_VALUE;
			for (Size size : sizes) {
				if (Math.abs(size.height - targetHeight) < minDiff
						&& size.width > w) {
					optimalSize = size;
					minDiff = Math.abs(size.height - targetHeight);
				}
			}
		}


			optimalSize = highest;


		params.setPictureSize(optimalSize.width, optimalSize.height);

		return optimalSize;
	}

	class afterPicture implements Camera.PictureCallback {
		private PassPicture pp;
		private Context context;
		private int height, width;

		public afterPicture(PassPicture p, Context c, int w, int h) {
			this.pp = p;
			context = c;
			width = w;
			height = h;
		}

		public void onPictureTaken(byte[] data, Camera camera) {
			myHandler.sendEmptyMessage(0);
			Log.d(TAG, "picture taken");
			Bitmap bmp = BitmapFactory.decodeByteArray(data, 0, data.length);

			try {
				File filesDir = context.getDir("cascade", Context.MODE_PRIVATE);
				File image = new File(filesDir, "temp.png");
				FileOutputStream out = new FileOutputStream(image);
				bmp.compress(Bitmap.CompressFormat.PNG, 50, out);

				String loc = image.getAbsolutePath();
				out.close();
				pp.setLocation(loc);
				
			} catch (Exception e) {
				e.printStackTrace();
			}

			progressBar.dismiss();
			Intent i = new Intent(context, VerifyActivity.class);
			context.startActivity(i);
			caller.finish(); 

		}
	}

	ShutterCallback shutterCallback = new ShutterCallback() {
		public void onShutter() {
			Log.d(TAG, "onShutter'd");
			Thread t = new Thread(new Runnable() {
				
				public void run() {
					try {
						Thread.sleep(3000);
						myHandler.sendEmptyMessage(1);
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
					
					
				}
			});
			t.start();
		}
	};

	/** Handles data for raw picture */
	PictureCallback rawCallback = new PictureCallback() {
		public void onPictureTaken(byte[] data, Camera camera) {
			if (data == null) {
				Log.d(TAG, "raw = null");
			}
			Log.d(TAG, "onPictureTaken - raw");
		}
	};

	/** Handles data for jpeg picture */
	PictureCallback jpegCallback = new PictureCallback() {
		public void onPictureTaken(byte[] data, Camera camera) {
			Log.d(TAG, "onPictureTaken - jpeg");
		}
	};

	/** Handles data for jpeg picture */
	PictureCallback post = new PictureCallback() {
		public void onPictureTaken(byte[] data, Camera camera) {
			Log.d(TAG, "onPictureTaken - jpeg");
		}
	};
	
	private Handler myHandler = new Handler() {
		boolean pictaken = false;
		
		@Override
		public void handleMessage(Message msg) {
			super.handleMessage(msg);
			
			// Go to loadingactivity if everything is complete.
			if(msg.what == 0){
				pictaken = true;		
			} 
			// Else display the login screen.
			if(msg.what == 1){
				
				if(pictaken){
					pictaken = false;
				} else {
					if(progressBar != null){
						progressBar.cancel();
					}
					
				}
			} 
		}
	};	

}
