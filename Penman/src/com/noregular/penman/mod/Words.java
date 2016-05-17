package com.noregular.penman.mod;

import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.core.MatOfRect;
import org.opencv.core.Rect;
import org.opencv.imgproc.Imgproc;

/**
 * Interface with native code to warp the textfield
 * 
 * @author Lucas Swartsenburg (6174388)
 * 
 */
public class Words {
	

	public Words() {
	}


	
	
	public Rect[] word(Mat color) {

		Mat out = new Mat();
		Mat grey = new Mat();
		Imgproc.cvtColor(color, grey, Imgproc.COLOR_RGBA2GRAY);
		
		nativeWord(color.getNativeObjAddr(), grey.getNativeObjAddr(), out.getNativeObjAddr());
		
		double[] temp = new double[1];
		Rect[] r = new Rect[out.rows()];
		for(int i = 0; i < out.rows(); i++){
			temp = out.get(i, 0);
			r[i].x = (int)temp[0];
			
			temp = out.get(i, 1);
			r[i].y = (int)temp[0];
			
			temp = out.get(i, 2);
			r[i].width = (int)temp[0];
			
			temp = out.get(i, 3);
			r[i].height = (int)temp[0];
			
			
			
		}
		
		return r;
	}
	
	public Mat cut(Rect r, Mat image){
		Mat result = new Mat();
		
		double in[] = new double[4];
		
		in[0] = r.x;
		in[1] = r.y;
		in[2] = r.width;
		in[3] = r.height;
		
		nativeCut(image.getNativeObjAddr(), in, result.getNativeObjAddr());
		
		return result;
	}
	
	private static native void nativeWord(long color, long grey, long out);
	
	private static native void nativeCut(long color, double in[], long out);

	/**
	 * Load the native library.
	 */
	static {
		System.loadLibrary("words");
	}
}
