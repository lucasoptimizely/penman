package com.noregular.penman.mod;

import org.opencv.core.Mat;

/**
 * Interface with native code to use the trained SVM to find triangles.
 * 
 * @author Lucas Swartsenburg (6174388)
 * 
 */
public class SVM {
	private long mNativeObj = 0;

	public SVM(String cascadeName) {
		mNativeObj = nativeCreateObject(cascadeName);
	}

	/**
	 * Detect triangles.
	 * 
	 * @param imageGray
	 *            The grey image that is searched for triangles
	 * @param imageColor
	 *            The color images that is the destination for output that
	 *            appears in the display.
	 * @param out
	 *            A mat object with 14 rows and 1 column that is used to
	 *            transfer parameters. The first value contains the minimum
	 *            width for taking a picture, the second the minumum height. The
	 *            other 12 values contains all values of the keypoints that are
	 *            found.
	 * @return True if a textfield is found. Only than the out mat is valid.
	 */
	public boolean detect(Mat imageGray, Mat imageColor, Mat out) {
		return nativeDetect(mNativeObj, imageGray.getNativeObjAddr(),
				imageColor.getNativeObjAddr(), out.getNativeObjAddr());
	}

	public boolean detect(Mat imageGray, Mat out) {
		return nativeDetect2(mNativeObj, imageGray.getNativeObjAddr(), out.getNativeObjAddr());
	}	
	
	/**
	 * Clean the SVM up after use.
	 */
	public void release() {
		nativeDestroyObject(mNativeObj);
		mNativeObj = 0;
	}

	private static native long nativeCreateObject(String cascadeName);

	private static native void nativeDestroyObject(long thiz);

	private static native boolean nativeDetect(long thiz, long grey,
			long color, long out);
	
	private static native boolean nativeDetect2(long thiz, long grey, long out);

	/**
	 * Load the native library.
	 */
	static {
		System.loadLibrary("svm");
	}
}
