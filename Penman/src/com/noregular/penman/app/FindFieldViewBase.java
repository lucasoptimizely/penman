package com.noregular.penman.app;
 
import java.io.IOException;
import java.util.List;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.PreviewCallback;
import android.os.Build;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

/**
 * The view that is used to capture textfields.
 * 
 * @author Lucas Swartsenburg (6174388)
 * 
 */
public abstract class FindFieldViewBase extends SurfaceView implements
		SurfaceHolder.Callback, Runnable {
	private static final String TAG = "FindFieldViewBase";

	protected Camera mCamera;
	private SurfaceHolder mHolder;
	private int mFrameWidth;
	private int mFrameHeight;
	private byte[] mFrame;
	private boolean mThreadRun;
	private byte[] mBuffer;

	/**
	 * Create a new view that searches contains the fields enabling the
	 * textfield search.
	 * 
	 * Source: OpenCV Tutorial 4 (Advanced) - Mix Java + Native OpenCV
	 * 
	 * @param context
	 */
	public FindFieldViewBase(Context context) {
		super(context);
		mHolder = getHolder();
		mHolder.addCallback(this);
		Log.i(TAG, "Instantiated new " + this.getClass());
	}

	public void clearMem(){
		mFrame = null;
		mBuffer = null;
		System.gc();
	}
	
	/**
	 * Returns the width of the frame.
	 * 
	 * Source: OpenCV Tutorial 4 (Advanced) - Mix Java + Native OpenCV
	 * 
	 * @return width of the frame in pixels
	 */
	public int getFrameWidth() {
		return mFrameWidth;
	}

	/**
	 * Returns the height of the frame.
	 * 
	 * Source: OpenCV Tutorial 4 (Advanced) - Mix Java + Native OpenCV
	 * 
	 * @return height of the frame in pixels
	 */
	public int getFrameHeight() {
		return mFrameHeight;
	}

	/**
	 * Sets the preview mode depending on the Android version.
	 * 
	 * Source: OpenCV Tutorial 4 (Advanced) - Mix Java + Native OpenCV
	 * 
	 * @throws IOException
	 */
	public void setPreview() throws IOException {
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB)
			mCamera.setPreviewTexture(new SurfaceTexture(10));
		else
			mCamera.setPreviewDisplay(null);
	}

	/**
	 * Starts the preview mode.
	 * 
	 * Source: OpenCV Tutorial 4 (Advanced) - Mix Java + Native OpenCV
	 * */
	public boolean openCamera() {
		Log.i(TAG, "openCamera");
		releaseCamera();
		mCamera = Camera.open();
		if (mCamera == null) {
			Log.e(TAG, "Can't open camera!");
			return false;
		}

		mCamera.setPreviewCallbackWithBuffer(new PreviewCallback() {
			public void onPreviewFrame(byte[] data, Camera camera) {
				synchronized (FindFieldViewBase.this) {
					System.arraycopy(data, 0, mFrame, 0, data.length);
					FindFieldViewBase.this.notify();
				}
				camera.addCallbackBuffer(mBuffer);
			}
		});
		return true;
	}

	/**
	 * Releases the camera. Used when the app is paused.
	 * 
	 * Source: OpenCV Tutorial 4 (Advanced) - Mix Java + Native OpenCV
	 * */
	public void releaseCamera() {
		Log.i(TAG, "releaseCamera");
		mThreadRun = false;
		synchronized (this) {
			if (mCamera != null) {
				mCamera.stopPreview();
				mCamera.setPreviewCallback(null);
				mCamera.release();
				mCamera = null;
			}
		}
		onPreviewStopped();
	}

	/**
	 * Sets all camera parameters. This includes determining the preview height
	 * and width using the screen size.
	 * 
	 * Source: OpenCV Tutorial 4 (Advanced) - Mix Java + Native OpenCV
	 * 
	 * @param width
	 * @param height
	 */
	public void setupCamera(int width, int height) {
		Log.i(TAG, "setupCamera");
		synchronized (this) {
			if (mCamera != null) {
				Camera.Parameters params = mCamera.getParameters();
				List<Camera.Size> sizes = params.getSupportedPreviewSizes();
				mFrameWidth = width;
				mFrameHeight = height;

				params.setPictureFormat(ImageFormat.JPEG);
				params.setJpegQuality(70);
				// selecting optimal camera preview size
				{
					int minDiff = Integer.MAX_VALUE;
					for (Camera.Size size : sizes) {
						if (Math.abs(size.height - height) < minDiff) {
							mFrameWidth = size.width;
							mFrameHeight = size.height;
							minDiff = Math.abs(size.height - height);
						}
					}
				}

				params.setPreviewSize(getFrameWidth(), getFrameHeight());

/*				List<String> FocusModes = params.getSupportedFocusModes();
				if (FocusModes
						.contains(Camera.Parameters.FOCUS_MODE_CONTINUOUS_VIDEO)) {
					params.setFocusMode(Camera.Parameters.FOCUS_MODE_CONTINUOUS_VIDEO);
				}*/

				mCamera.setParameters(params);

				/* Now allocate the buffer */
				params = mCamera.getParameters();
				int size = params.getPreviewSize().width
						* params.getPreviewSize().height;
				size = size
						* ImageFormat
								.getBitsPerPixel(params.getPreviewFormat()) / 8;
				mBuffer = new byte[size];
				/* The buffer where the current frame will be copied */
				mFrame = new byte[size];
				mCamera.addCallbackBuffer(mBuffer);

				try {
					setPreview();
				} catch (IOException e) {
					Log.e(TAG,
							"mCamera.setPreviewDisplay/setPreviewTexture fails: "
									+ e);
				}

				/*
				 * Notify that the preview is about to be started and deliver
				 * preview size
				 */
				onPreviewStarted(params.getPreviewSize().width,
						params.getPreviewSize().height);

				/* Now we can start a preview */
				mCamera.setErrorCallback(cb);
				mCamera.startPreview();
			}
		}
	}

	/**
	 * This is called immediately after any structural changes (format or size)
	 * have been made to the surface. This method is always called at least
	 * once, after surfaceCreated(SurfaceHolder). The function that sets all
	 * camera options is called.
	 * 
	 * Source: OpenCV Tutorial 4 (Advanced) - Mix Java + Native OpenCV
	 * 
	 * @param holder
	 *            The SurfaceHolder whose surface has changed.
	 * @param format
	 *            The new PixelFormat of the surface.
	 * @param width
	 *            The new width of the surface.
	 * @param height
	 *            The new height of the surface.
	 */
	public void surfaceChanged(SurfaceHolder _holder, int format, int width,
			int height) {
		Log.i(TAG, "surfaceChanged");
		setupCamera(width, height);
	}

	/**
	 * This is called immediately after the surface is first created. The thread
	 * where the camera is opened is started.
	 * 
	 * Source: OpenCV Tutorial 4 (Advanced) - Mix Java + Native OpenCV
	 * 
	 * @param holder
	 *            The SurfaceHolder whose surface has changed.
	 */
	public void surfaceCreated(SurfaceHolder holder) {
		Log.i(TAG, "surfaceCreated");
		(new Thread(this)).start();
	}

	/**
	 * This is called immediately before a surface is being destroyed. The
	 * camera is closed properly.
	 * 
	 * 
	 * Source: OpenCV Tutorial 4 (Advanced) - Mix Java + Native OpenCV
	 * 
	 * @param holder
	 *            The SurfaceHolder whose surface has changed.
	 */
	public void surfaceDestroyed(SurfaceHolder holder) {
		Log.i(TAG, "surfaceDestroyed");
		releaseCamera();
	}

	/**
	 * The bitmap returned by this method shall be owned by the child and
	 * released in onPreviewStopped().
	 * 
	 * Source: OpenCV Tutorial 4 (Advanced) - Mix Java + Native OpenCV
	 * 
	 * @param data
	 * @return
	 */
	protected abstract Bitmap processFrame(byte[] data);

	/**
	 * This method is called when the preview process is being started. It is
	 * called before the first frame delivered and processFrame is called It is
	 * called with the width and height parameters of the preview process. It
	 * can be used to prepare the data needed during the frame processing.
	 * 
	 * Source: OpenCV Tutorial 4 (Advanced) - Mix Java + Native OpenCV
	 * 
	 * @param previewWidth
	 *            - the width of the preview frames that will be delivered via
	 *            processFrame
	 * @param previewHeight
	 *            - the height of the preview frames that will be delivered via
	 *            processFrame
	 */
	protected abstract void onPreviewStarted(int previewWidtd, int previewHeight);

	/**
	 * This method is called when preview is stopped. When this method is called
	 * the preview stopped and all the processing of frames already completed.
	 * If the Bitmap object returned via processFrame is cached - it is a good
	 * time to recycle it. Any other resources used during the preview can be
	 * released.
	 * 
	 * Source: OpenCV Tutorial 4 (Advanced) - Mix Java + Native OpenCV
	 */
	protected abstract void onPreviewStopped();

	/** 
	 * This function runs in a thread and processes all preview frames. Thread save.
	 * 
	 * Source: OpenCV Tutorial 4 (Advanced) - Mix Java + Native OpenCV 
	 */
	public void run() {
		mThreadRun = true;
		Log.i(TAG, "Starting processing thread");
		while (mThreadRun) {
			Bitmap bmp = null;

			synchronized (this) {
				try {
					this.wait();
					bmp = processFrame(mFrame);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}

			if (bmp != null) {
				Canvas canvas = mHolder.lockCanvas();
				if (canvas != null) {
					canvas.drawBitmap(bmp,
							(canvas.getWidth() - getFrameWidth()) / 2,
							(canvas.getHeight() - getFrameHeight()) / 2, null);
					mHolder.unlockCanvasAndPost(canvas);
				}
			}
		}
	}
	
	Camera.ErrorCallback cb = new Camera.ErrorCallback() {
		
		public void onError(int error, Camera camera) {
			Log.d("camera error", error + "");
			
		}
	};
	
	
}