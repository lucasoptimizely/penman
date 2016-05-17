package com.noregular.penman.mod;

import org.opencv.core.Mat;

/**
 * Interface with native code to warp the textfield
 * 
 * @author Lucas Swartsenburg (6174388)
 * 
 */
public class WarpField {
	private long mNativeObj = 0;
	
	private Mat malt0;
	private Mat malt1;
	private Mat malt2;
	private Mat malt3;
	
	private int orientation = 0;

	public WarpField() {
	}


	public Mat warp(Mat color, Mat grey, double kps[]) {
		malt0 = new Mat();
		malt1 = new Mat();
		malt2 = new Mat();
		malt3 = new Mat();
		
			
		nativeWarp(color.getNativeObjAddr(), grey.getNativeObjAddr(), kps
				, malt0.getNativeObjAddr(), malt1.getNativeObjAddr(), malt2.getNativeObjAddr()
				, malt3.getNativeObjAddr());
		
		
		return malt0;
	}
	
	public Mat rotateLeft(){
		Mat result;
		if(orientation == 0){
			orientation = 3;
			result = malt3;
		} else if (orientation == 1){
			orientation = 0;
			result = malt0;
		} else if (orientation == 2){
			orientation = 1;
			result = malt1;
		} else {
			orientation = 2;
			result = malt2;
		}
		return result;
	}
	
	public Mat rotateRight(){
		Mat result;
		if(orientation == 0){
			orientation = 1; 
			result = malt1;
		} else if (orientation == 1){
			orientation = 2;
			result = malt2;
		} else if (orientation == 2){
			orientation = 3;
			result = malt3;
		} else {
			orientation = 0;
			result = malt0;
		}
		return result;		 
	}	
	
	public Mat getMat(){
		Mat result;
		if(orientation == 0){
			result = malt0;
		} else if (orientation == 1){
			result = malt1;
		} else if (orientation == 2){
			result = malt2;
		} else {
			result = malt3;
		}
		return result;			
	}


	private static native void nativeWarp(long color, long grey, double kps[], long alt0, long alt1, long alt2, long alt3);

	/**
	 * Load the native library.
	 */
	static {
		System.loadLibrary("warp");
	}
}
