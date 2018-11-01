/*
 * Copyright (c) 2017 Cisco and/or its affiliates.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 */
#include <config.h>

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500
#endif //_XOPEN_SOURCE 500

#ifndef __USE_XOPEN_EXTENDED
#define __USE_XOPEN_EXTENDED
#endif //__USE_XOPEN_EXTENDED

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>


#include <ftw.h>
#include <inttypes.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_PathName.h>
#include <parc/algol/parc_File.h>
#include <parc/algol/parc_Memory.h>


struct parc_file {
    PARCPathName *pathName;
};

static bool
_parcFile_Destructor(PARCFile **filePtr)
{
    PARCFile *file = *filePtr;

    parcPathName_Release(&file->pathName);

    return true;
}

parcObject_Override(PARCFile, PARCObject,
                    .destructor = (PARCObjectDestructor *) _parcFile_Destructor,
                    .toString = (PARCObjectToString *) parcFile_ToString);

void
parcFile_AssertValid(const PARCFile *instance)
{
    trapIllegalValueIf(instance == NULL, "Parameter must be a non-null pointer to a valid PARCFile.");
    trapIllegalValueIf(instance->pathName == NULL, "PARCFile cannot have a NULL path-name");
}

PARCFile *
parcFile_Create(const char *path)
{
    PARCFile *result = NULL;

    PARCPathName *pathName = parcPathName_Parse(path);
    if (pathName != NULL) {
        result = parcObject_CreateInstance(PARCFile);
        if (result != NULL) {
            result->pathName = pathName;
        } else {
            parcPathName_Release(&pathName);
        }
    }

    return result;
}

PARCFile *
parcFile_CreateChild(const PARCFile *parent, char *fileName)
{
    PARCFile *result = NULL;

    PARCPathName *childPath = parcPathName_Append(parcPathName_Copy(parent->pathName), fileName);

    if (childPath != NULL) {
        result = parcObject_CreateInstance(PARCFile);
        if (result != NULL) {
            result->pathName = childPath;
        }
    }

    return result;
}

parcObject_ImplementAcquire(parcFile, PARCFile);

parcObject_ImplementRelease(parcFile, PARCFile);

bool
parcFile_CreateNewFile(const PARCFile *file)
{
    parcFile_OptionalAssertValid(file);

    bool result = false;

    char *string = parcPathName_ToString(file->pathName);

    int fd = open(string, O_EXCL | O_CREAT | O_TRUNC, 0666);
    if (fd != -1) {
        close(fd);
        result = true;
    }
    parcMemory_Deallocate((void **) &string);

    return result;
}

bool
parcFile_Mkdir(const PARCFile *file)
{
    parcFile_OptionalAssertValid(file);

    char *string = parcPathName_ToString(file->pathName);
    bool result = (mkdir(string, 0777) == 0);
    parcMemory_Deallocate((void **) &string);

    return result;
}

bool
parcFile_Exists(const PARCFile *file)
{
    struct stat statbuf;

    char *string = parcPathName_ToString(file->pathName);
    bool result = stat(string, &statbuf) == 0;
    parcMemory_Deallocate((void **) &string);

    return result;
}

bool
parcFile_IsDirectory(const PARCFile *file)
{
    bool result = false;
    struct stat statbuf;

    char *string = parcPathName_ToString(file->pathName);
    if (stat(string, &statbuf) == 0) {
        result = S_ISDIR(statbuf.st_mode);
    }
    parcMemory_Deallocate((void **) &string);

    return result;
}

static int
_deleteNode(const char *path, const struct stat *stat, int flag, struct FTW *ftwbuf)
{
    int result = 0;

    if (flag == FTW_DP) { // directory in post-order
        result = rmdir(path);
    } else {
        result = unlink(path);
    }
    return result;
}

/**
 * @function parcFile_Delete
 * @abstract Deletes the file or directory
 * @discussion
 *
 * @param <#param1#>
 * @return true if everything deleted, false otherwise
 */
bool
parcFile_Delete(const PARCFile *file)
{
    char *string = parcPathName_ToString(file->pathName);

    // only allow under tmp
    assertTrue(strncmp(string, "/tmp/", 5) == 0,
               "Path must begin with /tmp/: %s", string);
    // dont allow ".."
    assertNull(strstr(string, ".."), "Path cannot have .. in it: %s", string);

    bool result = false;
    if (parcFile_IsDirectory(file)) {
        // depth first, dont't follow symlinks, do not cross mount points.
        int flags = FTW_DEPTH | FTW_PHYS | FTW_MOUNT;

        // maximum 20 fds open at a time
        int maximumFileDescriptors = 20;

        int failure = nftw(string, _deleteNode, maximumFileDescriptors, flags);
        assertFalse(failure, "Error on recursive delete: (%d) %s", errno, strerror(errno));

        result = failure == false;
    } else {
        result = (unlink(string) == 0);
    }

    parcMemory_Deallocate((void **) &string);

    return result;
}

PARCBufferComposer *
parcFile_BuildString(const PARCFile *file, PARCBufferComposer *composer)
{
    parcPathName_BuildString(file->pathName, composer);

    return composer;
}

char *
parcFile_ToString(const PARCFile *file)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();
    parcFile_BuildString(file, composer);

    PARCBuffer *tempBuffer = parcBufferComposer_ProduceBuffer(composer);
    char *result = parcBuffer_ToString(tempBuffer);
    parcBuffer_Release(&tempBuffer);

    parcBufferComposer_Release(&composer);

    return result;
}

size_t
parcFile_GetFileSize(const PARCFile *file)
{
    size_t fileSize = 0;

    char *fname = parcPathName_ToString(file->pathName);

    struct stat st;

    if (stat(fname, &st) == 0) {
        fileSize = st.st_size;
    }

    parcMemory_Deallocate(&fname);

    return fileSize;
}
