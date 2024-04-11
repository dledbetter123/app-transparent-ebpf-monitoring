#include <jvmti.h>
#include <stdio.h>
#include <string.h>

void JNICALL MethodEntryCallback(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread, jmethodID method) {
    char *name;
    char *signature;
    jvmtiError err;

    err = (*jvmti_env)->GetMethodName(jvmti_env, method, &name, &signature, NULL);
    if (err != JVMTI_ERROR_NONE) {
        printf("ERROR: Unable to retrieve method name and signature\n");
        return;
    }

    if (strcmp(name, "printInput") == 0 && strcmp(signature, "(Ljava/lang/String;)V") == 0) {
        jobject arg1;

        err = (*jvmti_env)->GetLocalObject(jvmti_env, thread, 0, 0, &arg1);
        if (err == JVMTI_ERROR_NONE && arg1 != NULL) {
            const char *strArg = (*jni_env)->GetStringUTFChars(jni_env, (jstring)arg1, NULL);
            if (strArg != NULL) {
                printf("printInput called with String argument: %s\n", strArg);
                (*jni_env)->ReleaseStringUTFChars(jni_env, (jstring)arg1, strArg);
            } else {
                printf("ERROR: GetStringUTFChars returned NULL\n");
            }
            (*jni_env)->DeleteLocalRef(jni_env, arg1);
        } else {
            printf("ERROR: Unable to get local object for method argument, error code: %d\n", err);
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
    capabilities.can_access_local_variables = 1;
    jvmtiError capErr = (*jvmti)->AddCapabilities(jvmti, &capabilities);
    if (capErr != JVMTI_ERROR_NONE) {
        printf("ERROR: Unable to add capabilities for accessing local variables, error code: %d\n", capErr);
        return JNI_ERR;
    }
    (*jvmti)->AddCapabilities(jvmti, &capabilities);

    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.MethodEntry = MethodEntryCallback;
    (*jvmti)->SetEventCallbacks(jvmti, &callbacks, sizeof(callbacks));

    (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE, JVMTI_EVENT_METHOD_ENTRY, NULL);

    return JNI_OK;
}