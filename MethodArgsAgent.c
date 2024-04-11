#include <jvmti.h>
#include <stdio.h>
#include <string.h>

void JNICALL MethodEntryCallback(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread, jmethodID method) {
    char *name;
    char *signature;


    (*jvmti_env)->GetMethodName(jvmti_env, method, &name, &signature, NULL);

    if (signature[1] == 'L' && strncmp(signature + 2, "java/lang/String;", 17) == 0) {

        jobject arg1;
        (*jvmti_env)->GetLocalObject(jvmti_env, thread, 0, 1, &arg1); // First argument

        const char *strArg = (*jni_env)->GetStringUTFChars(jni_env, (jstring)arg1, NULL);
        printf("Method %s called with String argument: %s\n", name, strArg);
        (*jni_env)->ReleaseStringUTFChars(jni_env, (jstring)arg1, strArg);
    }

    (*jvmti_env)->Deallocate(jvmti_env, (unsigned char*)name);
    (*jvmti_env)->Deallocate(jvmti_env, (unsigned char*)signature);
}

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *jvm, char *options, void *reserved) {
    jvmtiEnv *jvmti;
    jvmtiCapabilities capabilities;
    jvmtiEventCallbacks callbacks;

    (*jvm)->GetEnv(jvm, (void **)&jvmti, JVMTI_VERSION_1_0);

    memset(&capabilities, 0, sizeof(capabilities));
    capabilities.can_generate_method_entry_events = 1;
    (*jvmti)->AddCapabilities(jvmti, &capabilities);

    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.MethodEntry = MethodEntryCallback;
    (*jvmti)->SetEventCallbacks(jvmti, &callbacks, sizeof(callbacks));

    (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE, JVMTI_EVENT_METHOD_ENTRY, NULL);

    return JNI_OK;
}
