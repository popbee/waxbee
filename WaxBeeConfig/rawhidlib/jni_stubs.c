/* Simple Raw HID functions - JNI layer */

#include "rawhid.h"
#include "jni.h"

// JNI stubs
JNIEXPORT jint JNICALL Java_org_waxbee_RawHid_version(JNIEnv *env, jclass clazz)
{
	jint ret = 10;
	
	return ret;
}

JNIEXPORT jint JNICALL Java_org_waxbee_RawHid_open(JNIEnv *env, jclass clazz,
		jint max, jint vid, jint pid, jint usage_page, jint usage)
{
	return rawhid_open(max, vid, pid, usage_page, usage);
}

JNIEXPORT jint JNICALL Java_org_waxbee_RawHid_recv(JNIEnv *env, jclass clazz,
		jint num, jbyteArray bytes, jint timeout)
{
	jbyte *buf = (*env)->GetByteArrayElements(env, bytes, 0);
	jsize len = (*env)->GetArrayLength(env, bytes);

	int ret = rawhid_recv(num,buf,len,timeout);

	(*env)->ReleaseByteArrayElements(env, bytes, buf, 0);

	return ret;
}

JNIEXPORT jint JNICALL Java_org_waxbee_RawHid_send(JNIEnv *env, jclass clazz,
		jint num, jbyteArray bytes, jint timeout)
{
	jbyte *buf = (*env)->GetByteArrayElements(env, bytes, 0);
	jsize len = (*env)->GetArrayLength(env, bytes);

	int ret = rawhid_send(num,buf,len,timeout);
	
	(*env)->ReleaseByteArrayElements(env, bytes, buf, 0);
	
	return ret;
}

JNIEXPORT void JNICALL Java_org_waxbee_RawHid_close(JNIEnv *env, jclass clazz, jint num)
{
	rawhid_close(num);
}

JNIEXPORT void JNICALL Java_org_waxbee_RawHid_closeAll(JNIEnv *env, jclass clazz)
{
	rawhid_closeAll();
}

