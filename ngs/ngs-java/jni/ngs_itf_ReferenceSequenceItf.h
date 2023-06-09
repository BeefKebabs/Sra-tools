/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class ngs_itf_ReferenceSequenceItf */

#ifndef _Included_ngs_itf_ReferenceSequenceItf
#define _Included_ngs_itf_ReferenceSequenceItf
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     ngs_itf_ReferenceSequenceItf
 * Method:    GetCanonicalName
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_ngs_itf_ReferenceSequenceItf_GetCanonicalName
  (JNIEnv *, jobject, jlong);

/*
 * Class:     ngs_itf_ReferenceSequenceItf
 * Method:    GetIsCircular
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_ngs_itf_ReferenceSequenceItf_GetIsCircular
  (JNIEnv *, jobject, jlong);

/*
 * Class:     ngs_itf_ReferenceSequenceItf
 * Method:    GetLength
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_ngs_itf_ReferenceSequenceItf_GetLength
  (JNIEnv *, jobject, jlong);

/*
 * Class:     ngs_itf_ReferenceSequenceItf
 * Method:    GetReferenceBases
 * Signature: (JJJ)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_ngs_itf_ReferenceSequenceItf_GetReferenceBases
  (JNIEnv *, jobject, jlong, jlong, jlong);

/*
 * Class:     ngs_itf_ReferenceSequenceItf
 * Method:    GetReferenceChunk
 * Signature: (JJJ)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_ngs_itf_ReferenceSequenceItf_GetReferenceChunk
  (JNIEnv *, jobject, jlong, jlong, jlong);

#ifdef __cplusplus
}
#endif
#endif
