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
import org.opencv.core.Rect;
import org.opencv.features2d.KeyPoint;
import org.opencv.imgproc.Imgproc;

import com.noregular.penman.R;
import com.noregular.penman.ins.PassPicture;
import com.noregular.penman.mod.SVM;
import com.noregular.penman.mod.WarpField;
import com.noregular.penman.mod.Words;

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
import android.widget.ProgressBar;

/**
 * 
 * The activity that searches for the textfield and is able to capture it.
 * 
 * @author Lucas Swartsenburg (6174388)
 * 
 */
public class SendTextActivity extends Activity {
	static String TAG = "SendTextActivity";
	
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
					
					
					Words w = new Words();
					Rect rects[] = w.word(image);
					
					Log.v(TAG, rects.length + "");
					
					/*String body = "<img src=\"data:image/png;base64,";

					PassPicture pp = PassPicture.getInstance();
					Mat image = pp.getPicture();
					Bitmap bmp = Bitmap.createBitmap(image.cols(),
							image.rows(), Bitmap.Config.ARGB_8888);

					Utils.matToBitmap(image, bmp);

					//bmp = getResizedBitmap(bmp, 20, 20);

					ByteArrayOutputStream baos = new ByteArrayOutputStream();
					bmp.compress(Bitmap.CompressFormat.PNG, 20, baos);
					byte[] byteimage = baos.toByteArray();
					body += Base64.encodeToString(byteimage, Base64.DEFAULT);

					body += "\">";

					File dir = getDir("cascade", Context.MODE_WORLD_READABLE);
					File msg = new File(dir, "message.html");
					FileOutputStream fOut = new FileOutputStream(msg);
					OutputStreamWriter osw = new OutputStreamWriter(fOut);

					osw.write(body);

					osw.flush();
					osw.close();

					fOut.flush();
					fOut.close();

					
					 * //String path = msg.getAbsolutePath(); Uri uri =
					 * Uri.fromFile(msg);
					 * 
					 * 
					 * final Intent emailIntent = new Intent(
					 * android.content.Intent.ACTION_SEND);
					 * emailIntent.setType("text/html");
					 * 
					 * emailIntent.putExtra(android.content.Intent.EXTRA_STREAM,
					 * uri);
					 * 
					 * startActivity(Intent.createChooser(emailIntent,
					 * "Email:")); finish();
					 

					File root = Environment
							.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES);
					File save = new File(root, "message.png");
					InputStream input = new FileInputStream(msg);
					FileOutputStream os = new FileOutputStream(save);

					byte[] buffer = new byte[4096];
					int bytesRead;
					while ((bytesRead = input.read(buffer)) != -1) {
						os.write(buffer, 0, bytesRead);
					}
					input.close();
					os.close();

					// String path = msg.getAbsolutePath();
					Uri uri = Uri.fromFile(save);
					
					path = save.getAbsolutePath();

					final Intent emailIntent = new Intent(
							android.content.Intent.ACTION_SEND);
					emailIntent.setType("text/html");

					emailIntent.putExtra(android.content.Intent.EXTRA_STREAM,
							uri);

					startActivity(Intent.createChooser(emailIntent, "Email:"));*/

				} catch (Exception e) {
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
