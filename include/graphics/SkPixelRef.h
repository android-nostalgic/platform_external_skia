/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SkPixelRef_DEFINED
#define SkPixelRef_DEFINED

#include "SkRefCnt.h"
#include "SkString.h"

class SkColorTable;
class SkMutex;
class SkFlattenableReadBuffer;
class SkFlattenableWriteBuffer;

/** \class SkPixelRef

    This class is the smart container for pixel memory, and is used with
    SkBitmap. A pixelref is installed into a bitmap, and then the bitmap can
    access the actual pixel memory by calling lockPixels/unlockPixels.

    This class can be shared/accessed between multiple threads.
*/
class SkPixelRef : public SkRefCnt {
public:
    explicit SkPixelRef(SkMutex* mutex = NULL);
    
    /** Return the pixel memory returned from lockPixels, or null if the
        lockCount is 0.
    */
    void* pixels() const { return fPixels; }

    /** Return the current colorTable (if any) if pixels are locked, or null.
    */
    SkColorTable* colorTable() const { return fColorTable; }

    /** Return the current lockcount (defaults to 0)
    */
    int getLockCount() const { return fLockCount; }

    /** Call to access the pixel memory, which is returned. Balance with a call
        to unlockPixels().
    */
    void lockPixels();
    /** Call to balanace a previous call to lockPixels(). Returns the pixels
        (or null) after the unlock. NOTE: lock calls can be nested, but the
        matching number of unlock calls must be made in order to free the
        memory (if the subclass implements caching/deferred-decoding.)
    */
    void unlockPixels();
    
    /** Returns a non-zero, unique value corresponding to the pixels in this
        pixelref. Each time the pixels are changed (and notifyPixelsChanged is
        called), a different generation ID will be returned.
    */
    uint32_t getGenerationID() const;
    
    /** Call this if you have changed the contents of the pixels. This will in-
        turn cause a different generation ID value to be returned from
        getGenerationID().
    */
    void notifyPixelsChanged();

    /** Returns true if this pixelref is marked as immutable, meaning that the
        contents of its pixels will not change for the lifetime of the pixelref.
    */
    bool isImmutable() const { return fIsImmutable; }
    
    /** Marks this pixelref is immutable, meaning that the contents of its
        pixels will not change for the lifetime of the pixelref. This state can
        be set on a pixelref, but it cannot be cleared once it is set.
    */
    void setImmutable();

    /** Return the optional URI string associated with this pixelref. May be
        null.
    */
    const char* getURI() const { return fURI.size() ? fURI.c_str() : NULL; }

    /** Assign a URI string to this pixelref. If not null, the string is copied.
    */
    void setURI(const char uri[], size_t len = (size_t)-1) {
        fURI.set(uri, len);
    }
    
    /** Assign a URI string to this pixelref.
    */
    void setURI(const SkString& uri) { fURI = uri; }

    // serialization

    typedef SkPixelRef* (*Factory)(SkFlattenableReadBuffer&);

    virtual Factory getFactory() const { return NULL; }
    virtual void flatten(SkFlattenableWriteBuffer&) const;

    static Factory NameToFactory(const char name[]);
    static const char* FactoryToName(Factory);
    static void Register(const char name[], Factory);
    
    class Registrar {
    public:
        Registrar(const char name[], Factory factory) {
            SkPixelRef::Register(name, factory);
        }
    };

protected:
    /** Called when the lockCount goes from 0 to 1. The caller will have already
        acquire a mutex for thread safety, so this method need not do that.
    */
    virtual void* onLockPixels(SkColorTable**) = 0;
    /** Called when the lock count goes from 1 to 0. The caller will have
        already acquire a mutex for thread safety, so this method need not do
        that.
    */
    virtual void onUnlockPixels() = 0;

    /** Return the mutex associated with this pixelref. This value is assigned
        in the constructor, and cannot change during the lifetime of the object.
    */
    SkMutex* mutex() const { return fMutex; }

    SkPixelRef(SkFlattenableReadBuffer&, SkMutex*);

private:
    SkMutex*        fMutex; // must remain in scope for the life of this object
    void*           fPixels;
    SkColorTable*   fColorTable;    // we do not track ownership, subclass does
    int             fLockCount;
    
    mutable uint32_t fGenerationID;
    
    SkString    fURI;

    // can go from false to true, but never from true to false
    bool    fIsImmutable;
};

#endif
