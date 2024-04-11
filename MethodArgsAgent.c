#include <jvmti.h>
#include <stdio.h>
#include <string.h>

void JNICALL MethodEntryCallback(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread, jmethodID method) {
    char *name;
    char *signature;
    jvmtiError err;

    err = (*jvmti_env)->GetMethodName(jvmti_env, method, &name, &signature, NULL);
    if (err != JVMTI_ERROR_NONE) {

        return;
    }

    // when signature is NULL it causes segmentation faults in the JVM
    if (signature != NULL && strstr(name, "print") != NULL) {
        jobject arg1;
        printf("Method with 'print' in its name: %s\n", name);

        // get the first local object, safeguarded by checks
        err = (*jvmti_env)->GetLocalObject(jvmti_env, thread, 0, 1, &arg1);
        if (err == JVMTI_ERROR_NONE && arg1 != NULL) {
            const char *strArg = (*jni_env)->GetStringUTFChars(jni_env, (jstring)arg1, NULL);
            if (strArg != NULL) {
                printf("Method %s called with String argument: %s\n", name, strArg);
                (*jni_env)->ReleaseStringUTFChars(jni_env, (jstring)arg1, strArg);
            }
            // Release local reference to avoid memory leak
            (*jni_env)->DeleteLocalRef(jni_env, arg1);
        } else {
            print(arg1);
        }
    }

    if (name != NULL) {
        (*jvmti_env)->Deallocate(jvmti_env, (unsigned char*)name);
    }
    if (signature != NULL) {
        (*jvmti_env)->Deallocate(jvmti_env, (unsigned char*)signature);
    }
}

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *jvm, char *options, void *reserved) {
    jvmtiEnv *jvmti;
    jvmtiCapabilities capabilities;
    jvmtiEventCallbacks callbacks;
    jvmtiError err;

    jint res = (*jvm)->GetEnv(jvm, (void **)&jvmti, JVMTI_VERSION_1_0);
    if (res != JNI_OK || jvmti == NULL) {
        // JNI_OK (0) indicates success; anything else is an error
        return JNI_ERR;
    }

    memset(&capabilities, 0, sizeof(capabilities));
    capabilities.can_generate_method_entry_events = 1;
    (*jvmti)->AddCapabilities(jvmti, &capabilities);

    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.MethodEntry = MethodEntryCallback;
    (*jvmti)->SetEventCallbacks(jvmti, &callbacks, sizeof(callbacks));

    (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE, JVMTI_EVENT_METHOD_ENTRY, NULL);

    return JNI_OK;
}
