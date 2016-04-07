#ifndef CDC_VERSION_H
#define CDC_VERSION_H

#ifdef __cplusplus
extern "C" {
#endif

#define REPO_TAG "CedarC-2.6-v0.2"
#define REPO_BRANCH "master"
#define REPO_COMMIT "0ac3eef64fa4dc07195ed3dd4db8e2fa6d6b0b29"
#define REPO_DATE "Fri Mar 25 14:27:51 2016 +0800"
#define RELEASE_AUTHOR "jenkins"

static inline void LogVersionInfo(void)
{
    logd("\n"
         ">>>>>>>>>>>>>>>>>>>>>>>>>>>>> Cedar Codec <<<<<<<<<<<<<<<<<<<<<<<<<<<<\n" 
         "tag   : %s\n"
         "branch: %s\n"
         "commit: %s\n"
         "date  : %s\n"
         "author: %s\n"
         "----------------------------------------------------------------------\n",
         REPO_TAG, REPO_BRANCH, REPO_COMMIT, REPO_DATE, RELEASE_AUTHOR);
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

