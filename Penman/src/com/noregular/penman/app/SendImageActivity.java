package com.noregular.penman.app;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStreamWriter;

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
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Bitmap.Config;
import android.graphics.Matrix;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.renderscript.Type;
import android.text.Html;
import android.util.Base64;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ProgressBar;

/**
 * 
 * The activity that searches for the textfield and is able to capture it.
 * 
 * @author Lucas Swartsenburg (6174388)
 * 
 */
public class SendImageActivity extends Activity {
	static String TAG = "SendImageActivity";
	
	private String path;

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setUI();

	}

	public void setUI() {
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
				WindowManager.LayoutParams.FLAG_FULLSCREEN);
		setContentView(R.layout.generateimage);
		sendImage();

	}

	public void sendImage() {
		Thread thread = new Thread() {
			@Override
			public void run() {
				try {

					PassPicture pp = PassPicture.getInstance();
					Mat image = pp.getPicture();
					Bitmap bmp = Bitmap.createBitmap(image.cols(),
							image.rows(), Bitmap.Config.ARGB_8888);

					Utils.matToBitmap(image, bmp);

					//bmp = getResizedBitmap(bmp, 20, 20);

					File root = Environment
							.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES);
					File msg = new File(root, "message.png");
					
					FileOutputStream out = new FileOutputStream(msg);
					
					bmp.compress(Bitmap.CompressFormat.PNG, 20, out);
					
					out.close();
					
					// String path = msg.getAbsolutePath();
					Uri uri = Uri.fromFile(msg);
					
					path = msg.getAbsolutePath();

					final Intent emailIntent = new Intent(
							android.content.Intent.ACTION_SEND);
					emailIntent.setType("text/html");
					
					String body = "\n\n-- Picture added using Penman --";

					emailIntent.putExtra(android.content.Intent.EXTRA_STREAM,
							uri);
					emailIntent.putExtra(android.content.Intent.EXTRA_TEXT, body);
					
					startActivity(Intent.createChooser(emailIntent, "Email:"));

				} catch (IOException e) {
					e.printStackTrace();
					Log.e(TAG, "Failed to load cascade. Exception thrown: " + e);
				}

			}
		};
		thread.start();
	}

	public void onRestart(){
		
		File file = new File(path);
		file.delete();
		super.onRestart();
/*		LinearLayout prog = (LinearLayout) findViewById(R.id.generateprogress);
		LinearLayout butt = (LinearLayout) findViewById(R.id.generatebuttons);
		
		prog.setVisibility(View.INVISIBLE);
		butt.setVisibility(View.VISIBLE);
		
		Button fini = (Button) findViewById(R.id.generatefinish);
		Button back = (Button) findViewById(R.id.generateback);*/
		finish();
		
	}
	
	public Bitmap getResizedBitmap(Bitmap bm, int newHeight, int newWidth) {

		int width = bm.getWidth();

		int height = bm.getHeight();

		float scaleWidth = ((float) newWidth) / width;

		float scaleHeight = ((float) newHeight) / height;

		// create a matrix for the manipulation

		Matrix matrix = new Matrix();

		// resize the bit map

		matrix.postScale(scaleWidth, scaleHeight);

		// recreate the new Bitmap

		Bitmap resizedBitmap = Bitmap.createBitmap(bm, 0, 0, width, height,
				matrix, false);

		return resizedBitmap;

	}

}
