package com.noregular.penman.app;

import java.io.ByteArrayInputStream;
import java.io.InputStream;

import org.opencv.android.Utils;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.features2d.KeyPoint;
import org.opencv.imgproc.Imgproc;

import com.noregular.penman.R;
import com.noregular.penman.ins.PassPicture;
import com.noregular.penman.mod.SVM;
import com.noregular.penman.mod.WarpField;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Bitmap.Config;
import android.os.Bundle;
import android.renderscript.Type;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.ProgressBar;

/**
 * 
 * The activity that searches for the textfield and is able to capture it.
 * 
 * @author Lucas Swartsenburg (6174388)
 * 
 */
public class VerifyActivity extends Activity {
	static String TAG = "VerifyActivity";

	private String msg = "";
	
	private WarpField warp = new WarpField();
	
	private Bitmap bmp;

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setUI();

		loadImage();

	}

	public void setUI() {
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
				WindowManager.LayoutParams.FLAG_FULLSCREEN);
		setContentView(R.layout.showimage2);

	}

	public void loadImage() {
		Thread thread = new Thread() {
			@Override
			public void run() {
				try {

					PassPicture pp = PassPicture.getInstance();
					SVM svm = new SVM(pp.getSVM());
					bmp = BitmapFactory.decodeFile(pp.getLocation());
					Log.v(TAG,
							"Bmp dims: " + bmp.getWidth() + "x"
									+ bmp.getHeight());
					KeyPoint kps[] = pp.getK();

					Mat mat = new Mat();
					Utils.bitmapToMat(bmp, mat);

					Mat grey = new Mat();
					Imgproc.cvtColor(mat, grey, Imgproc.COLOR_RGBA2GRAY);
					Mat out = new Mat(14, 1, CvType.CV_64FC1);

					if (!svm.detect(grey, out)) {
						msg = "Sorry, I couldn't find four triangles in the picture. Please move as "
								+ "little as you can until you hear the shutter sound.";
						runOnUiThread(new Runnable() {
							public void run() {
								onBackPressed();
							}
						});
					} else {
						Log.v(TAG,
								"Mat mat dims: " + mat.cols() + "x"
										+ mat.rows());
						bmp = null;
						System.gc();
						double temp[];
						temp = out.get(0, 0);
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
						double kpsarr[] = new double[12];
						for (int i = 0; i < 4; i++) {
							int r = i * 3;
							kpsarr[r] = kps[i].size;
							kpsarr[r + 1] = kps[i].pt.x;
							kpsarr[r + 2] = kps[i].pt.y;
						}
						Mat w = warp.warp(mat, grey, kpsarr);

						Log.v(TAG,
								"Warp mat dims: " + w.cols() + "x"
										+ w.rows());

						bmp = Bitmap.createBitmap(w.cols(), w.rows(),
								Bitmap.Config.ARGB_8888);

						Utils.matToBitmap(w, bmp);

						final Bitmap result = bmp;

						runOnUiThread(new Runnable() {
							public void run() {
								ProgressBar pr = (ProgressBar) findViewById(R.id.progressBar1);
								pr.setVisibility(View.INVISIBLE);

								ImageView im = (ImageView) findViewById(R.id.imageView1);
								im.setImageBitmap(result);

								activateButtons();
							}
						});
						bmp = null;

					}
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		};

		thread.start();
	}

	public void activateButtons() {
		ImageButton left = (ImageButton) findViewById(R.id.imageButton1);
		left.setOnClickListener(new View.OnClickListener() {
			public void onClick(View arg0) {
				rotateRight();
			}
		});
		ImageButton right = (ImageButton) findViewById(R.id.imageButton2);
		right.setOnClickListener(new View.OnClickListener() {
			public void onClick(View arg0) {
				rotateLeft();
			}
		});

		Button im = (Button) findViewById(R.id.sendasimage);
		im.setOnClickListener(new View.OnClickListener() {
			public void onClick(View arg0) {
				sendAsImage();
			}
		});

		Button wo = (Button) findViewById(R.id.sendaswords);
		wo.setOnClickListener(new View.OnClickListener() {
			public void onClick(View arg0) {
				sendAsText();
			}
		});
	}

	public void rotateLeft() {
		System.gc();
		Mat w = warp.rotateLeft();
		bmp = Bitmap.createBitmap(w.cols(), w.rows(),
				Bitmap.Config.ARGB_8888);
		Utils.matToBitmap(w, bmp);
		final Bitmap result = bmp;
		runOnUiThread(new Runnable() {
			public void run() {
				ImageView im = (ImageView) findViewById(R.id.imageView1);
				im.setImageBitmap(result);
			}
		});
		bmp = null;
	}

	public void rotateRight() {
		System.gc();
		Mat w = warp.rotateRight();
		bmp = Bitmap.createBitmap(w.cols(), w.rows(),
				Bitmap.Config.ARGB_8888);
		Utils.matToBitmap(w, bmp); 
		final Bitmap result = bmp;
		runOnUiThread(new Runnable() {
			public void run() {
				ImageView im = (ImageView) findViewById(R.id.imageView1);
				im.setImageBitmap(result);
			}
		});
		bmp = null;		

	}

	public void sendAsImage() {
		PassPicture pp = PassPicture.getInstance();
		pp.clear();
		pp.setPicture(warp.getMat());
		Intent i = new Intent(VerifyActivity.this, SendImageActivity.class);
		startActivity(i);
		finish();

	}

	public void sendAsText() {
		PassPicture pp = PassPicture.getInstance();
		pp.clear();
		pp.setPicture(warp.getMat());
		Intent i = new Intent(VerifyActivity.this, SendTextActivity.class);
		startActivity(i);
		finish();
	}

	public void onBackPressed() {
		Intent i = new Intent(VerifyActivity.this, FindFieldActivity.class);
		i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		i.putExtra("msg", msg);
		startActivity(i);

		super.onBackPressed();
		finish();
	}

	public void onResume() {
		msg = "";
		super.onResume();
	}

}
