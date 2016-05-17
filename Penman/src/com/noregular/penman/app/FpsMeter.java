package com.noregular.penman.app;

import java.text.DecimalFormat;

import org.opencv.core.Core;

import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.util.Log;



/**
 * A Frames Per Second meter.
 * 
 * @author OpenCV Sample - face-detection
 */
public class FpsMeter {
	private static final String TAG = "Fps meter";
	int step;
	int framesCouner;
	double freq;
	long prevFrameTime;
	String strfps;
	DecimalFormat twoPlaces = new DecimalFormat("0.00");
	Paint paint;

	/**
	 * Initiates a Frames Per Second meter.
	 * 
	 * Source: OpenCV Sample - face-detection
	 */
	public FpsMeter() {
		step = 20;
		framesCouner = 0;
		freq = Core.getTickFrequency();
		prevFrameTime = Core.getTickCount();
		strfps = "";

		paint = new Paint();
		paint.setColor(Color.BLUE);
		paint.setTextSize(50);
	}

	/**
	 * Counts frames and prints the framerate every 20 frames.
	 * 
	 * Source: OpenCV Sample - face-detection
	 */
	public void measure() {
		framesCouner++;
		if (framesCouner % step == 0) {
			long time = Core.getTickCount();
			double fps = step * freq / (time - prevFrameTime);
			prevFrameTime = time;
			DecimalFormat twoPlaces = new DecimalFormat("0.00");
			strfps = twoPlaces.format(fps) + " FPS";
			Log.i(TAG, strfps);
		}
	}

	/**
	 * Draws the framerate on the provided canvas
	 * 
	 * Source: OpenCV Sample - face-detection
	 * 
	 * @param canvas
	 * @param offsetx
	 * @param offsety
	 */
	public void draw(Canvas canvas, float offsetx, float offsety) {
		canvas.drawText(strfps, 20 + offsetx, 10 + 50 + offsety, paint);
	}

}
