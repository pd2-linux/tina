#ifndef CDX_VERSION_H
#define CDX_VERSION_H

#ifdef __cplusplus
extern "C" {
#endif

#define REPO_TAG "cedarx_linux_v0.7"
#define REPO_BRANCH "master"
#define REPO_COMMIT "6064dd49ef9dbe181a240d537f3e804966d3382d"
#define RELEASE_AUTHOR "xuqi"

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


