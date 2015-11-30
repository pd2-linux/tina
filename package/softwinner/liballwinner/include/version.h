#ifndef CDX_VERSION_H
#define CDX_VERSION_H

#ifdef __cplusplus
extern "C" {
#endif

#define REPO_TAG "patch0010-cedarx2.3"
#define REPO_BRANCH "master"
#define REPO_COMMIT "688cf9db77eec1af142efc4b245c40c05f3d9264"
#define RELEASE_AUTHOR "guoshaomin"

static inline void LogVersionInfo(void)
{
    logd("\n"
         ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> CedarX <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n" 
         "tag   : %s\n"
         "branch: %s\n"
         "commit: %s\n"
         "author: %s\n"
         "----------------------------------------------------------------------\n",
         REPO_TAG, REPO_BRANCH, REPO_COMMIT, RELEASE_AUTHOR);
}

/* usage: TagVersionInfo(myLibTag) */
#define TagVersionInfo(tag) \
    static void VersionInfo_##tag(void) __attribute__((constructor));\
    void VersionInfo_##tag(void) \
    { \
        logd("-------library tag: %s-------", #tag);\
        LogVersionInfo(); \
    }


#ifdef __cplusplus
}
#endif

#endif


