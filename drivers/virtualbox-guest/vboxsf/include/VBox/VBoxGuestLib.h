/** @file
 * VBoxGuestLib - VirtualBox Guest Additions Library.
 */

/*
 * Copyright (C) 2006-2019 Oracle Corporation
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef VBOX_INCLUDED_VBoxGuestLib_h
#define VBOX_INCLUDED_VBoxGuestLib_h
#ifndef RT_WITHOUT_PRAGMA_ONCE
# pragma once
#endif

#include <VBox/types.h>
#include <VBox/VMMDev.h>
#include <VBox/VBoxGuestCoreTypes.h>
# ifdef VBOX_WITH_DRAG_AND_DROP
#  include <VBox/GuestHost/DragAndDropDefs.h>
# endif

/** @defgroup grp_vboxguest_lib     VirtualBox Guest Additions Library
 * @ingroup grp_vboxguest
 * @{
 */

/** @page pg_guest_lib  VirtualBox Guest Library
 *
 * This is a library for abstracting the additions driver interface. There are
 * multiple versions of the library depending on the context. The main
 * distinction is between kernel and user mode where the interfaces are very
 * different.
 *
 *
 * @section sec_guest_lib_ring0     Ring-0
 *
 * In ring-0 there are two version:
 *  - VBOX_LIB_VBGL_R0_BASE / VBoxGuestR0LibBase for the VBoxGuest main driver,
 *    who is responsible for managing the VMMDev virtual hardware.
 *  - VBOX_LIB_VBGL_R0 / VBoxGuestR0Lib for other (client) guest drivers.
 *
 *
 * The library source code and the header have a define VBGL_VBOXGUEST, which is
 * defined for VBoxGuest and undefined for other drivers. Drivers must choose
 * right library in their makefiles and set VBGL_VBOXGUEST accordingly.
 *
 * The libraries consists of:
 *  - common code to be used by both VBoxGuest and other drivers;
 *  - VBoxGuest specific code;
 *  - code for other drivers which communicate with VBoxGuest via an IOCTL.
 *
 *
 * @section sec_guest_lib_ring3     Ring-3
 *
 * There are more variants of the library here:
 *  - VBOX_LIB_VBGL_R3 / VBoxGuestR3Lib for programs.
 *  - VBOX_LIB_VBGL_R3_XFREE86 / VBoxGuestR3LibXFree86 for old style XFree
 *    drivers which uses special loader and or symbol resolving strategy.
 *  - VBOX_LIB_VBGL_R3_SHARED / VBoxGuestR3LibShared for shared objects / DLLs /
 *    Dylibs.
 *
 */

RT_C_DECLS_BEGIN

/** HGCM client ID.
 * @todo Promote to VBox/types.h  */
typedef uint32_t HGCMCLIENTID;


/** @defgroup grp_vboxguest_lib_r0     Ring-0 interface.
 * @{
 */
#ifdef IN_RING0
/** @def DECLR0VBGL
 * Declare a VBGL ring-0 API with the right calling convention and visibilitiy.
 * @param type      Return type.  */
# ifdef RT_OS_DARWIN /** @todo probably apply to all, but don't want a forest fire on our hands right now. */
#  define DECLR0VBGL(type) DECLHIDDEN(type) VBOXCALL
# else
#  define DECLR0VBGL(type) type VBOXCALL
# endif
# define DECLVBGL(type) DECLR0VBGL(type)


/**
 * The library initialization function to be used by the main VBoxGuest driver.
 *
 * @return VBox status code.
 */
DECLR0VBGL(int)     VbglR0InitPrimary(RTIOPORT portVMMDev, VMMDevMemory *pVMMDevMemory, uint32_t *pfFeatures);

/**
 * The library termination function to be used by the main VBoxGuest driver.
 *
 * @author bird (2017-08-23)
 */
DECLR0VBGL(void)    VbglR0TerminatePrimary(void);

/**
 * The library initialization function to be used by all drivers
 * other than the main VBoxGuest system driver.
 *
 * @return VBox status code.
 */
DECLR0VBGL(int)     VbglR0InitClient(void);

/**
 * The library termination function.
 */
DECLR0VBGL(void)    VbglR0TerminateClient(void);

/**
 * Query the host feature mask.
 *
 * @returns VBox status code.
 * @param   pfHostFeatures      Where to return the host feature mask,
 *                              VMMDEV_HVF_XXX.
 * @note    Client only. May fail we're unable to connect VBoxGuest.
 */
DECLR0VBGL(int)     VbglR0QueryHostFeatures(uint32_t *pfHostFeatures);


/** @name The IDC Client Interface
 * @{
 */

/**
 * Inter-Driver Communication Handle.
 */
typedef union VBGLIDCHANDLE
{
    /** Padding for opaque usage.
     * Must be greater or equal in size than the private struct. */
    void *apvPadding[4];
#ifdef VBGLIDCHANDLEPRIVATE_DECLARED
    /** The private view. */
    struct VBGLIDCHANDLEPRIVATE s;
#endif
} VBGLIDCHANDLE;
/** Pointer to a handle. */
typedef VBGLIDCHANDLE *PVBGLIDCHANDLE;

DECLR0VBGL(int) VbglR0IdcOpen(PVBGLIDCHANDLE pHandle, uint32_t uReqVersion, uint32_t uMinVersion,
                              uint32_t *puSessionVersion, uint32_t *puDriverVersion, uint32_t *puDriverRevision);
struct VBGLREQHDR;
DECLR0VBGL(int) VbglR0IdcCallRaw(PVBGLIDCHANDLE pHandle, uintptr_t uReq, struct VBGLREQHDR *pReqHdr, uint32_t cbReq);
DECLR0VBGL(int) VbglR0IdcCall(PVBGLIDCHANDLE pHandle, uintptr_t uReq, struct VBGLREQHDR *pReqHdr, uint32_t cbReq);
DECLR0VBGL(int) VbglR0IdcClose(PVBGLIDCHANDLE pHandle);

/** @} */


/** @name Generic request functions.
 * @{
 */

/**
 * Allocate memory for generic request and initialize the request header.
 *
 * @returns VBox status code.
 * @param   ppReq       Where to return the pointer to the allocated memory.
 * @param   cbReq       Size of memory block required for the request.
 * @param   enmReqType  the generic request type.
 */
# if defined(VBOX_INCLUDED_VMMDev_h) || defined(DOXYGEN_RUNNING)
DECLR0VBGL(int) VbglR0GRAlloc(struct VMMDevRequestHeader **ppReq, size_t cbReq, VMMDevRequestType enmReqType);
# else
DECLR0VBGL(int) VbglR0GRAlloc(struct VMMDevRequestHeader **ppReq, size_t cbReq, int32_t enmReqType);
# endif

/**
 * Perform the generic request.
 *
 * @param pReq     pointer the request structure.
 *
 * @return VBox status code.
 */
DECLR0VBGL(int) VbglR0GRPerform(struct VMMDevRequestHeader *pReq);

/**
 * Free the generic request memory.
 *
 * @param pReq     pointer the request structure.
 *
 * @return VBox status code.
 */
DECLR0VBGL(void) VbglR0GRFree(struct VMMDevRequestHeader *pReq);

/**
 * Verify the generic request header.
 *
 * @param pReq     pointer the request header structure.
 * @param cbReq    size of the request memory block. It should be equal to the request size
 *                 for fixed size requests. It can be greater than the request size for
 *                 variable size requests.
 *
 * @return VBox status code.
 */
DECLR0VBGL(int) VbglGR0Verify(const struct VMMDevRequestHeader *pReq, size_t cbReq);

/** @} */

# ifdef VBOX_WITH_HGCM
struct VBGLIOCHGCMCALL;
struct VBGLIOCIDCHGCMFASTCALL;

#  ifdef VBGL_VBOXGUEST

/**
 * Callback function called from HGCM helpers when a wait for request
 * completion IRQ is required.
 *
 * @returns VINF_SUCCESS, VERR_INTERRUPT or VERR_TIMEOUT.
 * @param   pvData      VBoxGuest pointer to be passed to callback.
 * @param   u32Data     VBoxGuest 32 bit value to be passed to callback.
 */
typedef DECLCALLBACK(int) FNVBGLHGCMCALLBACK(VMMDevHGCMRequestHeader *pHeader, void *pvData, uint32_t u32Data);
/** Pointer to a FNVBGLHGCMCALLBACK. */
typedef FNVBGLHGCMCALLBACK *PFNVBGLHGCMCALLBACK;

/**
 * Perform a connect request.
 *
 * That is locate required service and obtain a client identifier for future
 * access.
 *
 * @note This function can NOT handle cancelled requests!
 *
 * @param   pLoc                The service to connect to.
 * @param   fRequestor          VMMDEV_REQUESTOR_XXX.
 * @param   pidClient           Where to return the client ID on success.
 * @param   pfnAsyncCallback    Required pointer to function that is calledwhen
 *                              host returns VINF_HGCM_ASYNC_EXECUTE. VBoxGuest
 *                              implements waiting for an IRQ in this function.
 * @param   pvAsyncData         An arbitrary VBoxGuest pointer to be passed to callback.
 * @param   u32AsyncData        An arbitrary VBoxGuest 32 bit value to be passed to callback.
 *
 * @return  VBox status code.
 */
DECLR0VBGL(int) VbglR0HGCMInternalConnect(HGCMServiceLocation const *pLoc, uint32_t fRequestor, HGCMCLIENTID *pidClient,
                                          PFNVBGLHGCMCALLBACK pfnAsyncCallback, void *pvAsyncData, uint32_t u32AsyncData);


/**
 * Perform a disconnect request.
 *
 * That is tell the host that the client will not call the service anymore.
 *
 * @note This function can NOT handle cancelled requests!
 *
 * @param   idClient            The client ID to disconnect.
 * @param   fRequestor          VMMDEV_REQUESTOR_XXX.
 * @param   pfnAsyncCallback    Required pointer to function that is called when
 *                              host returns VINF_HGCM_ASYNC_EXECUTE. VBoxGuest
 *                              implements waiting for an IRQ in this function.
 * @param   pvAsyncData         An arbitrary VBoxGuest pointer to be passed to callback.
 * @param   u32AsyncData        An arbitrary VBoxGuest 32 bit value to be passed to
 *                              callback.
 *
 * @return  VBox status code.
 */

DECLR0VBGL(int) VbglR0HGCMInternalDisconnect(HGCMCLIENTID idClient, uint32_t fRequestor,
                                             PFNVBGLHGCMCALLBACK pfnAsyncCallback, void *pvAsyncData, uint32_t u32AsyncData);

/** Call a HGCM service.
 *
 * @note This function can deal with cancelled requests.
 *
 * @param   pCallInfo           The request data.
 * @param   fFlags              Flags, see VBGLR0_HGCMCALL_F_XXX.
 * @param   fRequestor          VMMDEV_REQUESTOR_XXX.
 * @param   pfnAsyncCallback    Required pointer to function that is called when
 *                              host returns VINF_HGCM_ASYNC_EXECUTE. VBoxGuest
 *                              implements waiting for an IRQ in this function.
 * @param   pvAsyncData         An arbitrary VBoxGuest pointer to be passed to callback.
 * @param   u32AsyncData        An arbitrary VBoxGuest 32 bit value to be passed to callback.
 *
 * @return VBox status code.
 */
DECLR0VBGL(int) VbglR0HGCMInternalCall(struct VBGLIOCHGCMCALL *pCallInfo, uint32_t cbCallInfo, uint32_t fFlags, uint32_t fRequestor,
                                       PFNVBGLHGCMCALLBACK pfnAsyncCallback, void *pvAsyncData, uint32_t u32AsyncData);

/** Call a HGCM service. (32 bits packet structure in a 64 bits guest)
 *
 * @note This function can deal with cancelled requests.
 *
 * @param   pCallInfo           The request data.
 * @param   fFlags              Flags, see VBGLR0_HGCMCALL_F_XXX.
 * @param   fRequestor          VMMDEV_REQUESTOR_XXX.
 * @param   pfnAsyncCallback    Required pointer to function that is called when
 *                              host returns VINF_HGCM_ASYNC_EXECUTE. VBoxGuest
 *                              implements waiting for an IRQ in this function.
 * @param   pvAsyncData         An arbitrary VBoxGuest pointer to be passed to callback.
 * @param   u32AsyncData        An arbitrary VBoxGuest 32 bit value to be passed to callback.
 *
 * @return  VBox status code.
 */
DECLR0VBGL(int) VbglR0HGCMInternalCall32(struct VBGLIOCHGCMCALL *pCallInfo, uint32_t cbCallInfo, uint32_t fFlags, uint32_t fRequestor,
                                         PFNVBGLHGCMCALLBACK pfnAsyncCallback, void *pvAsyncData, uint32_t u32AsyncData);

/** @name VbglR0HGCMInternalCall flags
 * @{ */
/** User mode request.
 * Indicates that only user mode addresses are permitted as parameters. */
#define VBGLR0_HGCMCALL_F_USER          UINT32_C(0)
/** Kernel mode request.
 * Indicates that kernel mode addresses are permitted as parameters. Whether or
 * not user mode addresses are permitted is, unfortunately, OS specific. The
 * following OSes allows user mode addresses: Windows, TODO.
 */
#define VBGLR0_HGCMCALL_F_KERNEL        UINT32_C(1)
/** Mode mask. */
#define VBGLR0_HGCMCALL_F_MODE_MASK     UINT32_C(1)
/** @} */

#  else  /* !VBGL_VBOXGUEST */

#ifndef VBGL_VBOXGUEST
/** @internal  */
typedef struct VBGLHGCMHANDLEDATA
{
    uint32_t fAllocated;
    VBGLIDCHANDLE IdcHandle;
} VBGLHGCMHANDLEDATA;
#else
struct VBGLHGCMHANDLEDATA;
#endif

typedef struct VBGLHGCMHANDLEDATA *VBGLHGCMHANDLE;

/** @name HGCM functions
 * @{
 */

/**
 * Initializes HGCM in the R0 guest library.  Must be called before any HGCM
 * connections are made.  Is called by VbglInitClient().
 *
 * @return VBox status code.
 */
DECLR0VBGL(int) VbglR0HGCMInit(void);

/**
 * Terminates HGCM in the R0 guest library.  Is called by VbglTerminate().
 *
 * @return VBox status code.
 */
DECLR0VBGL(int) VbglR0HGCMTerminate(void);

/**
 * Connect to a service.
 *
 * @param pHandle           Pointer to variable that will hold a handle to be used
 *                          further in VbglHGCMCall and VbglHGCMClose.
 * @param pszServiceName    The service to connect to.
 * @param pidClient         Where to return the client ID for the connection.
 *
 * @return VBox status code.
 *
 * @todo consider baking the client Id into the handle.
 */
DECLR0VBGL(int) VbglR0HGCMConnect(VBGLHGCMHANDLE *pHandle, const char *pszServiceName, HGCMCLIENTID *pidClient);

/**
 * Connect to a service.
 *
 * @param handle      Handle of the connection.
 * @param idClient    The ID of the client connection.
 *
 * @return VBox status code.
 *
 * @todo consider baking the client Id into the handle.
 */
DECLR0VBGL(int) VbglR0HGCMDisconnect(VBGLHGCMHANDLE handle, HGCMCLIENTID idClient);

/**
 * Call to a service, returning only the I/O control status code.
 *
 * @param handle      Handle of the connection.
 * @param pData       Call request information structure, including function parameters.
 * @param cbData      Length in bytes of data.
 *
 * @return VBox status code.
 */
DECLR0VBGL(int) VbglR0HGCMCallRaw(VBGLHGCMHANDLE handle, struct VBGLIOCHGCMCALL *pData, uint32_t cbData);

/**
 * Call to a service, returning the HGCM status code.
 *
 * @param handle      Handle of the connection.
 * @param pData       Call request information structure, including function parameters.
 * @param cbData      Length in bytes of data.
 *
 * @return VBox status code.  Either the I/O control status code if that failed,
 *         or the HGCM status code (pData->Hdr.rc).
 */
DECLR0VBGL(int) VbglR0HGCMCall(VBGLHGCMHANDLE handle, struct VBGLIOCHGCMCALL *pData, uint32_t cbData);

/**
 * Call to a service with user-mode data received by the calling driver from the User-Mode process.
 * The call must be done in the context of a calling process.
 *
 * @param handle      Handle of the connection.
 * @param pData       Call request information structure, including function parameters.
 * @param cbData      Length in bytes of data.
 *
 * @return VBox status code.
 */
DECLR0VBGL(int) VbglR0HGCMCallUserDataRaw(VBGLHGCMHANDLE handle, struct VBGLIOCHGCMCALL *pData, uint32_t cbData);

/**
 * Call to a service, w/o any repacking and buffer locking in VBoxGuest,
 * returning the only request related status code (not HGCM).
 *
 * The driver only submits the request and waits for completion, nothing else.
 *
 * @param   hHandle     The connection handle.
 * @param   pCallReq    The call request.  Will be passed directly to the host.
 * @param   cbCallReq   The size of the whole call request.
 *
 * @return VBox status code.
 *
 * @remarks The result of the HGCM call is found in
 *          @a pCallReq->HgcmCallReq.header.result on a successful return.  The
 *          @a pCallReq->Hdr.rc and @a pCallReq->HgcmCallReq.header.header.rc
 *          fields are the same as the return value and can safely be ignored.
 */
DECLR0VBGL(int) VbglR0HGCMFastCall(VBGLHGCMHANDLE hHandle, struct VBGLIOCIDCHGCMFASTCALL *pCallReq, uint32_t cbCallReq);

/** @} */

/** @name Undocumented helpers for talking to the Chromium OpenGL Host Service
 * @{ */
typedef VBGLHGCMHANDLE VBGLCRCTLHANDLE;
DECLR0VBGL(int) VbglR0CrCtlCreate(VBGLCRCTLHANDLE *phCtl);
DECLR0VBGL(int) VbglR0CrCtlDestroy(VBGLCRCTLHANDLE hCtl);
DECLR0VBGL(int) VbglR0CrCtlConConnect(VBGLCRCTLHANDLE hCtl, HGCMCLIENTID *pidClient);
DECLR0VBGL(int) VbglR0CrCtlConDisconnect(VBGLCRCTLHANDLE hCtl, HGCMCLIENTID idClient);
struct VBGLIOCHGCMCALL;
DECLR0VBGL(int) VbglR0CrCtlConCallRaw(VBGLCRCTLHANDLE hCtl, struct VBGLIOCHGCMCALL *pCallInfo, int cbCallInfo);
DECLR0VBGL(int) VbglR0CrCtlConCall(VBGLCRCTLHANDLE hCtl, struct VBGLIOCHGCMCALL *pCallInfo, int cbCallInfo);
DECLR0VBGL(int) VbglR0CrCtlConCallUserDataRaw(VBGLCRCTLHANDLE hCtl, struct VBGLIOCHGCMCALL *pCallInfo, int cbCallInfo);
/** @} */

#  endif /* !VBGL_VBOXGUEST */

# endif /* VBOX_WITH_HGCM */


/**
 * Initialize the heap.
 *
 * @returns VBox status code.
 */
DECLR0VBGL(int)     VbglR0PhysHeapInit(void);

/**
 * Shutdown the heap.
 */
DECLR0VBGL(void)    VbglR0PhysHeapTerminate(void);

/**
 * Allocate a memory block.
 *
 * @returns Virtual address of the allocated memory block.
 * @param cbSize    Size of block to be allocated.
 */
DECLR0VBGL(void *)  VbglR0PhysHeapAlloc(uint32_t cbSize);

/**
 * Get physical address of memory block pointed by the virtual address.
 *
 * @note WARNING!
 *       The function does not acquire the Heap mutex!
 *       When calling the function make sure that the pointer is a valid one and
 *       is not being deallocated.  This function can NOT be used for verifying
 *       if the given pointer is a valid one allocated from the heap.
 *
 * @param   pv      Virtual address of memory block.
 * @returns Physical address of the memory block.
 */
DECLR0VBGL(uint32_t) VbglR0PhysHeapGetPhysAddr(void *pv);

/**
 * Free a memory block.
 *
 * @param   pv    Virtual address of memory block.
 */
DECLR0VBGL(void)    VbglR0PhysHeapFree(void *pv);

DECLR0VBGL(int)     VbglR0QueryVMMDevMemory(struct VMMDevMemory **ppVMMDevMemory);
DECLR0VBGL(bool)    VbglR0CanUsePhysPageList(void);

# ifndef VBOX_GUEST
/** @name Mouse
 * @{ */
DECLR0VBGL(int)     VbglR0SetMouseNotifyCallback(PFNVBOXGUESTMOUSENOTIFY pfnNotify, void *pvUser);
DECLR0VBGL(int)     VbglR0GetMouseStatus(uint32_t *pfFeatures, uint32_t *px, uint32_t *py);
DECLR0VBGL(int)     VbglR0SetMouseStatus(uint32_t fFeatures);
/** @}  */
# endif /* VBOX_GUEST */

#endif /* IN_RING0 */

/** @} */


/** @defgroup grp_vboxguest_lib_r3      Ring-3 interface.
 * @{
 */
#ifdef IN_RING3

/** @def VBGLR3DECL
 * Ring 3 VBGL declaration.
 * @param   type    The return type of the function declaration.
 */
# define VBGLR3DECL(type) DECLHIDDEN(type) VBOXCALL

/** @name General-purpose functions
 * @{ */
VBGLR3DECL(int)     VbglR3Init(void);
VBGLR3DECL(int)     VbglR3InitUser(void);
VBGLR3DECL(void)    VbglR3Term(void);
# ifdef IPRT_INCLUDED_time_h
VBGLR3DECL(int)     VbglR3GetHostTime(PRTTIMESPEC pTime);
# endif
VBGLR3DECL(int)     VbglR3InterruptEventWaits(void);
VBGLR3DECL(int)     VbglR3WriteLog(const char *pch, size_t cch);
VBGLR3DECL(int)     VbglR3CtlFilterMask(uint32_t fOr, uint32_t fNot);
VBGLR3DECL(int)     VbglR3Daemonize(bool fNoChDir, bool fNoClose, bool fRespawn, unsigned *pcRespawn);
VBGLR3DECL(int)     VbglR3PidFile(const char *pszPath, PRTFILE phFile);
VBGLR3DECL(void)    VbglR3ClosePidFile(const char *pszPath, RTFILE hFile);
VBGLR3DECL(int)     VbglR3SetGuestCaps(uint32_t fOr, uint32_t fNot);
VBGLR3DECL(int)     VbglR3AcquireGuestCaps(uint32_t fOr, uint32_t fNot, bool fConfig);
VBGLR3DECL(int)     VbglR3WaitEvent(uint32_t fMask, uint32_t cMillies, uint32_t *pfEvents);

VBGLR3DECL(int)     VbglR3ReportAdditionsStatus(VBoxGuestFacilityType Facility, VBoxGuestFacilityStatus StatusCurrent,
                                                uint32_t fFlags);
VBGLR3DECL(int)     VbglR3GetAdditionsVersion(char **ppszVer, char **ppszVerEx, char **ppszRev);
VBGLR3DECL(int)     VbglR3GetAdditionsInstallationPath(char **ppszPath);
VBGLR3DECL(int)     VbglR3GetSessionId(uint64_t *pu64IdSession);

/** @} */

/** @name Shared clipboard
 * @{ */
VBGLR3DECL(int)     VbglR3ClipboardConnect(HGCMCLIENTID *pidClient);
VBGLR3DECL(int)     VbglR3ClipboardDisconnect(HGCMCLIENTID idClient);
VBGLR3DECL(int)     VbglR3ClipboardGetHostMsg(HGCMCLIENTID idClient, uint32_t *pMsg, uint32_t *pfFormats);
VBGLR3DECL(int)     VbglR3ClipboardReadData(HGCMCLIENTID idClient, uint32_t fFormat, void *pv, uint32_t cb, uint32_t *pcb);
VBGLR3DECL(int)     VbglR3ClipboardReportFormats(HGCMCLIENTID idClient, uint32_t fFormats);
VBGLR3DECL(int)     VbglR3ClipboardWriteData(HGCMCLIENTID idClient, uint32_t fFormat, void *pv, uint32_t cb);
/** @} */

/** @name Seamless mode
 * @{ */
VBGLR3DECL(int)     VbglR3SeamlessSetCap(bool fState);
VBGLR3DECL(int)     VbglR3SeamlessWaitEvent(VMMDevSeamlessMode *pMode);
VBGLR3DECL(int)     VbglR3SeamlessSendRects(uint32_t cRects, PRTRECT pRects);
VBGLR3DECL(int)     VbglR3SeamlessGetLastEvent(VMMDevSeamlessMode *pMode);

/** @}  */

/** @name Mouse
 * @{ */
VBGLR3DECL(int)     VbglR3GetMouseStatus(uint32_t *pfFeatures, uint32_t *px, uint32_t *py);
VBGLR3DECL(int)     VbglR3SetMouseStatus(uint32_t fFeatures);
/** @}  */

/** @name Video
 * @{ */
VBGLR3DECL(int)     VbglR3VideoAccelEnable(bool fEnable);
VBGLR3DECL(int)     VbglR3VideoAccelFlush(void);
VBGLR3DECL(int)     VbglR3SetPointerShape(uint32_t fFlags, uint32_t xHot, uint32_t yHot, uint32_t cx, uint32_t cy,
                                          const void *pvImg, size_t cbImg);
VBGLR3DECL(int)     VbglR3SetPointerShapeReq(struct VMMDevReqMousePointer *pReq);
/** @}  */

/** @name Display
 * @{ */
/** The folder for the video mode hint unix domain socket on Unix-like guests.
 * @note This can be safely changed as all users are rebuilt in lock-step. */
#define VBGLR3HOSTDISPSOCKETPATH    "/tmp/.VBoxService"
/** The path to the video mode hint unix domain socket on Unix-like guests. */
#define VBGLR3HOSTDISPSOCKET        VBGLR3VIDEOMODEHINTSOCKETPATH "/VideoModeHint"

/** The folder for saving video mode hints to between sessions. */
#define VBGLR3HOSTDISPSAVEDMODEPATH "/var/lib/VBoxGuestAdditions"
/** The path to the file for saving video mode hints to between sessions. */
#define VBGLR3HOSTDISPSAVEDMODE     VBGLR3HOSTDISPSAVEDMODEPATH "/SavedVideoModes"

VBGLR3DECL(int)     VbglR3GetDisplayChangeRequest(uint32_t *pcx, uint32_t *pcy, uint32_t *pcBits, uint32_t *piDisplay,
                                                  uint32_t *pdx, uint32_t *pdy, bool *pfEnabled, bool *pfChangeOrigin, bool fAck);
VBGLR3DECL(int)     VbglR3GetDisplayChangeRequestMulti(uint32_t cDisplaysIn, uint32_t *pcDisplaysOut,
                                                       VMMDevDisplayDef *paDisplays, bool fAck);
VBGLR3DECL(bool)    VbglR3HostLikesVideoMode(uint32_t cx, uint32_t cy, uint32_t cBits);
VBGLR3DECL(int)     VbglR3VideoModeGetHighestSavedScreen(unsigned *pcScreen);
VBGLR3DECL(int)     VbglR3SaveVideoMode(unsigned cScreen, unsigned cx, unsigned cy, unsigned cBits,
                                        unsigned x, unsigned y, bool fEnabled);
VBGLR3DECL(int)     VbglR3RetrieveVideoMode(unsigned cScreen, unsigned *pcx, unsigned *pcy, unsigned *pcBits,
                                            unsigned *px, unsigned *py, bool *pfEnabled);
/** @} */

/** @name VRDP
 * @{ */
VBGLR3DECL(int)     VbglR3VrdpGetChangeRequest(bool *pfActive, uint32_t *puExperienceLevel);
/** @} */

/** @name VM Statistics
 * @{ */
VBGLR3DECL(int)     VbglR3StatQueryInterval(uint32_t *pu32Interval);
# if defined(VBOX_INCLUDED_VMMDev_h) || defined(DOXYGEN_RUNNING)
VBGLR3DECL(int)     VbglR3StatReport(VMMDevReportGuestStats *pReq);
# endif
/** @}  */

/** @name Memory ballooning
 * @{ */
VBGLR3DECL(int)     VbglR3MemBalloonRefresh(uint32_t *pcChunks, bool *pfHandleInR3);
VBGLR3DECL(int)     VbglR3MemBalloonChange(void *pv, bool fInflate);
/** @}  */

/** @name Core Dump
 * @{ */
VBGLR3DECL(int)     VbglR3WriteCoreDump(void);

/** @}  */

# ifdef VBOX_WITH_GUEST_PROPS
/** @name Guest properties
 * @{ */
/** @todo Docs. */
typedef struct VBGLR3GUESTPROPENUM VBGLR3GUESTPROPENUM;
/** @todo Docs. */
typedef VBGLR3GUESTPROPENUM *PVBGLR3GUESTPROPENUM;
VBGLR3DECL(int)     VbglR3GuestPropConnect(uint32_t *pidClient);
VBGLR3DECL(int)     VbglR3GuestPropDisconnect(HGCMCLIENTID idClient);
VBGLR3DECL(int)     VbglR3GuestPropWrite(HGCMCLIENTID idClient, const char *pszName, const char *pszValue, const char *pszFlags);
VBGLR3DECL(int)     VbglR3GuestPropWriteValue(HGCMCLIENTID idClient, const char *pszName, const char *pszValue);
VBGLR3DECL(int)     VbglR3GuestPropWriteValueV(HGCMCLIENTID idClient, const char *pszName,
                                               const char *pszValueFormat, va_list va) RT_IPRT_FORMAT_ATTR(3, 0);
VBGLR3DECL(int)     VbglR3GuestPropWriteValueF(HGCMCLIENTID idClient, const char *pszName,
                                               const char *pszValueFormat, ...) RT_IPRT_FORMAT_ATTR(3, 4);
VBGLR3DECL(int)     VbglR3GuestPropRead(HGCMCLIENTID idClient, const char *pszName, void *pvBuf, uint32_t cbBuf, char **ppszValue,
                                        uint64_t *pu64Timestamp, char **ppszFlags, uint32_t *pcbBufActual);
VBGLR3DECL(int)     VbglR3GuestPropReadValue(uint32_t ClientId, const char *pszName, char *pszValue, uint32_t cchValue,
                                             uint32_t *pcchValueActual);
VBGLR3DECL(int)     VbglR3GuestPropReadValueAlloc(HGCMCLIENTID idClient, const char *pszName, char **ppszValue);
VBGLR3DECL(void)    VbglR3GuestPropReadValueFree(char *pszValue);
VBGLR3DECL(int)     VbglR3GuestPropEnumRaw(HGCMCLIENTID idClient, const char *paszPatterns, char *pcBuf, uint32_t cbBuf,
                                           uint32_t *pcbBufActual);
VBGLR3DECL(int)     VbglR3GuestPropEnum(HGCMCLIENTID idClient, char const * const *ppaszPatterns, uint32_t cPatterns,
                                        PVBGLR3GUESTPROPENUM *ppHandle, char const **ppszName, char const **ppszValue,
                                        uint64_t *pu64Timestamp, char const **ppszFlags);
VBGLR3DECL(int)     VbglR3GuestPropEnumNext(PVBGLR3GUESTPROPENUM pHandle, char const **ppszName, char const **ppszValue,
                                            uint64_t *pu64Timestamp, char const **ppszFlags);
VBGLR3DECL(void)    VbglR3GuestPropEnumFree(PVBGLR3GUESTPROPENUM pHandle);
VBGLR3DECL(int)     VbglR3GuestPropDelete(HGCMCLIENTID idClient, const char *pszName);
VBGLR3DECL(int)     VbglR3GuestPropDelSet(HGCMCLIENTID idClient, char const * const *papszPatterns, uint32_t cPatterns);
VBGLR3DECL(int)     VbglR3GuestPropWait(HGCMCLIENTID idClient, const char *pszPatterns, void *pvBuf, uint32_t cbBuf,
                                        uint64_t u64Timestamp, uint32_t cMillies, char ** ppszName, char **ppszValue,
                                        uint64_t *pu64Timestamp, char **ppszFlags, uint32_t *pcbBufActual);
/** @}  */

/** @name Guest user handling / reporting.
 * @{ */
VBGLR3DECL(int)     VbglR3GuestUserReportState(const char *pszUser, const char *pszDomain, VBoxGuestUserState enmState,
                                               uint8_t *pbDetails, uint32_t cbDetails);
/** @}  */

/** @name Host version handling
 * @{ */
VBGLR3DECL(int)     VbglR3HostVersionCheckForUpdate(HGCMCLIENTID idClient, bool *pfUpdate, char **ppszHostVersion,
                                                    char **ppszGuestVersion);
VBGLR3DECL(int)     VbglR3HostVersionLastCheckedLoad(HGCMCLIENTID idClient, char **ppszVer);
VBGLR3DECL(int)     VbglR3HostVersionLastCheckedStore(HGCMCLIENTID idClient, const char *pszVer);
/** @}  */
# endif /* VBOX_WITH_GUEST_PROPS defined */

# ifdef VBOX_WITH_SHARED_FOLDERS
/** @name Shared folders
 * @{ */
/**
 * Structure containing mapping information for a shared folder.
 */
typedef struct VBGLR3SHAREDFOLDERMAPPING
{
    /** Mapping status. */
    uint32_t u32Status;
    /** Root handle. */
    uint32_t u32Root;
} VBGLR3SHAREDFOLDERMAPPING;
/** Pointer to a shared folder mapping information structure. */
typedef VBGLR3SHAREDFOLDERMAPPING *PVBGLR3SHAREDFOLDERMAPPING;
/** Pointer to a const shared folder mapping information structure. */
typedef VBGLR3SHAREDFOLDERMAPPING const *PCVBGLR3SHAREDFOLDERMAPPING;

VBGLR3DECL(int)     VbglR3SharedFolderConnect(uint32_t *pidClient);
VBGLR3DECL(int)     VbglR3SharedFolderDisconnect(HGCMCLIENTID idClient);
VBGLR3DECL(bool)    VbglR3SharedFolderExists(HGCMCLIENTID idClient, const char *pszShareName);
VBGLR3DECL(int)     VbglR3SharedFolderGetMappings(HGCMCLIENTID idClient, bool fAutoMountOnly,
                                                  PVBGLR3SHAREDFOLDERMAPPING *ppaMappings, uint32_t *pcMappings);
VBGLR3DECL(void)    VbglR3SharedFolderFreeMappings(PVBGLR3SHAREDFOLDERMAPPING paMappings);
VBGLR3DECL(int)     VbglR3SharedFolderGetName(HGCMCLIENTID  idClient,uint32_t u32Root, char **ppszName); /**< @todo r=bird: GET functions return the value, not a status code!*/
VBGLR3DECL(int)     VbglR3SharedFolderQueryFolderInfo(HGCMCLIENTID idClient, uint32_t idRoot, uint64_t fQueryFlags,
                                                      char **ppszName, char **ppszMountPoint,
                                                      uint64_t *pfFlags, uint32_t *puRootIdVersion);
VBGLR3DECL(int)     VbglR3SharedFolderWaitForMappingsChanges(HGCMCLIENTID idClient, uint32_t uPrevVersion, uint32_t *puCurVersion);
VBGLR3DECL(int)     VbglR3SharedFolderCancelMappingsChangesWaits(HGCMCLIENTID idClient);

VBGLR3DECL(int)     VbglR3SharedFolderGetMountPrefix(char **ppszPrefix); /**< @todo r=bird: GET functions return the value, not a status code! */
VBGLR3DECL(int)     VbglR3SharedFolderGetMountDir(char **ppszDir);       /**< @todo r=bird: GET functions return the value, not a status code! */
/** @}  */
# endif /* VBOX_WITH_SHARED_FOLDERS defined */

# ifdef VBOX_WITH_GUEST_CONTROL
/** @name Guest control
 * @{ */

/**
 * Structure containing the context required for
 * either retrieving or sending a HGCM guest control
 * commands from or to the host.
 *
 * Note: Do not change parameter order without also
 *       adapting all structure initializers.
 */
typedef struct VBGLR3GUESTCTRLCMDCTX
{
    /** @todo This struct could be handy if we want to implement
     *        a second communication channel, e.g. via TCP/IP.
     *        Use a union for the HGCM stuff then. */

    /** IN: HGCM client ID to use for communication. */
    uint32_t uClientID;
    /** IN/OUT: Context ID to retrieve or to use. */
    uint32_t uContextID;
    /** IN: Protocol version to use. */
    uint32_t uProtocol;
    /** OUT: Number of parameters retrieved. */
    uint32_t uNumParms;
} VBGLR3GUESTCTRLCMDCTX, *PVBGLR3GUESTCTRLCMDCTX;

/* General message handling on the guest. */
VBGLR3DECL(int) VbglR3GuestCtrlConnect(uint32_t *pidClient);
VBGLR3DECL(int) VbglR3GuestCtrlDisconnect(uint32_t idClient);
VBGLR3DECL(bool) VbglR3GuestCtrlSupportsOptimizations(uint32_t idClient);
VBGLR3DECL(int) VbglR3GuestCtrlMakeMeMaster(uint32_t idClient);
VBGLR3DECL(int) VbglR3GuestCtrlReportFeatures(uint32_t idClient, uint64_t fGuestFeatures, uint64_t *pfHostFeatures);
VBGLR3DECL(int) VbglR3GuestCtrlQueryFeatures(uint32_t idClient, uint64_t *pfHostFeatures);
VBGLR3DECL(int) VbglR3GuestCtrlMsgFilterSet(uint32_t uClientId, uint32_t uValue, uint32_t uMaskAdd, uint32_t uMaskRemove);
VBGLR3DECL(int) VbglR3GuestCtrlMsgReply(PVBGLR3GUESTCTRLCMDCTX pCtx, int rc);
VBGLR3DECL(int) VbglR3GuestCtrlMsgReplyEx(PVBGLR3GUESTCTRLCMDCTX pCtx, int rc, uint32_t uType,
                                          void *pvPayload, uint32_t cbPayload);
VBGLR3DECL(int) VbglR3GuestCtrlMsgSkip(uint32_t idClient, int rcSkip, uint32_t idMsg);
VBGLR3DECL(int) VbglR3GuestCtrlMsgSkipOld(uint32_t uClientId);
VBGLR3DECL(int) VbglR3GuestCtrlMsgPeekWait(uint32_t idClient, uint32_t *pidMsg, uint32_t *pcParameters, uint64_t *pidRestoreCheck);
VBGLR3DECL(int) VbglR3GuestCtrlCancelPendingWaits(HGCMCLIENTID idClient);
/* Guest session handling. */
VBGLR3DECL(int) VbglR3GuestCtrlSessionPrepare(uint32_t idClient, uint32_t idSession, void const *pvKey, uint32_t cbKey);
VBGLR3DECL(int) VbglR3GuestCtrlSessionAccept(uint32_t idClient, uint32_t idSession, void const *pvKey, uint32_t cbKey);
VBGLR3DECL(int) VbglR3GuestCtrlSessionCancelPrepared(uint32_t idClient, uint32_t idSession);
VBGLR3DECL(int) VbglR3GuestCtrlSessionClose(PVBGLR3GUESTCTRLCMDCTX pCtx, uint32_t fFlags);
VBGLR3DECL(int) VbglR3GuestCtrlSessionNotify(PVBGLR3GUESTCTRLCMDCTX pCtx, uint32_t uType, int32_t iResult);
VBGLR3DECL(int) VbglR3GuestCtrlSessionGetOpen(PVBGLR3GUESTCTRLCMDCTX pCtx, uint32_t *puProtocol, char *pszUser, uint32_t cbUser,
                                              char *pszPassword, uint32_t  cbPassword, char *pszDomain, uint32_t cbDomain,
                                              uint32_t *pfFlags, uint32_t *pidSession);
VBGLR3DECL(int) VbglR3GuestCtrlSessionGetClose(PVBGLR3GUESTCTRLCMDCTX pCtx, uint32_t *pfFlags, uint32_t *pidSession);
/* Guest path handling. */
VBGLR3DECL(int) VbglR3GuestCtrlPathGetRename(PVBGLR3GUESTCTRLCMDCTX pCtx, char *pszSource, uint32_t cbSource, char *pszDest,
                                             uint32_t cbDest, uint32_t *pfFlags);
VBGLR3DECL(int) VbglR3GuestCtrlPathGetUserDocuments(PVBGLR3GUESTCTRLCMDCTX pCtx);
VBGLR3DECL(int) VbglR3GuestCtrlPathGetUserHome(PVBGLR3GUESTCTRLCMDCTX pCtx);
/* Guest process execution. */
VBGLR3DECL(int) VbglR3GuestCtrlProcGetStart(PVBGLR3GUESTCTRLCMDCTX pCtx, char *pszCmd, uint32_t cbCmd, uint32_t *pfFlags,
                                            char *pszArgs, uint32_t cbArgs, uint32_t *puNumArgs, char *pszEnv, uint32_t *pcbEnv,
                                            uint32_t *puNumEnvVars, char *pszUser, uint32_t cbUser, char *pszPassword,
                                            uint32_t cbPassword, uint32_t *puTimeoutMS, uint32_t *puPriority,
                                            uint64_t *puAffinity, uint32_t cbAffinity, uint32_t *pcAffinity);
VBGLR3DECL(int) VbglR3GuestCtrlProcGetTerminate(PVBGLR3GUESTCTRLCMDCTX pCtx, uint32_t *puPID);
VBGLR3DECL(int) VbglR3GuestCtrlProcGetInput(PVBGLR3GUESTCTRLCMDCTX pCtx, uint32_t *puPID, uint32_t *pfFlags, void *pvData,
                                            uint32_t cbData, uint32_t *pcbSize);
VBGLR3DECL(int) VbglR3GuestCtrlProcGetOutput(PVBGLR3GUESTCTRLCMDCTX pCtx, uint32_t *puPID, uint32_t *puHandle, uint32_t *pfFlags);
VBGLR3DECL(int) VbglR3GuestCtrlProcGetWaitFor(PVBGLR3GUESTCTRLCMDCTX pCtx, uint32_t *puPID, uint32_t *puWaitFlags,
                                              uint32_t *puTimeoutMS);
/* Guest native directory handling. */
VBGLR3DECL(int) VbglR3GuestCtrlDirGetRemove(PVBGLR3GUESTCTRLCMDCTX pCtx, char *pszPath, uint32_t cbPath, uint32_t *pfFlags);
/* Guest native file handling. */
VBGLR3DECL(int) VbglR3GuestCtrlFileGetOpen(PVBGLR3GUESTCTRLCMDCTX pCtx, char *pszFileName, uint32_t cbFileName, char *pszOpenMode,
                                           uint32_t cbOpenMode, char *pszDisposition, uint32_t cbDisposition, char *pszSharing,
                                           uint32_t cbSharing, uint32_t *puCreationMode, uint64_t *puOffset);
VBGLR3DECL(int) VbglR3GuestCtrlFileGetClose(PVBGLR3GUESTCTRLCMDCTX pCtx, uint32_t *puHandle);
VBGLR3DECL(int) VbglR3GuestCtrlFileGetRead(PVBGLR3GUESTCTRLCMDCTX pCtx, uint32_t *puHandle, uint32_t *puToRead);
VBGLR3DECL(int) VbglR3GuestCtrlFileGetReadAt(PVBGLR3GUESTCTRLCMDCTX pCtx, uint32_t *puHandle,
                                             uint32_t *puToRead, uint64_t *poffRead);
VBGLR3DECL(int) VbglR3GuestCtrlFileGetWrite(PVBGLR3GUESTCTRLCMDCTX pCtx, uint32_t *puHandle,
                                            void *pvData, uint32_t cbData, uint32_t *pcbActual);
VBGLR3DECL(int) VbglR3GuestCtrlFileGetWriteAt(PVBGLR3GUESTCTRLCMDCTX pCtx, uint32_t *puHandle, void *pvData, uint32_t cbData,
                                              uint32_t *pcbActual, uint64_t *poffWrite);
VBGLR3DECL(int) VbglR3GuestCtrlFileGetSeek(PVBGLR3GUESTCTRLCMDCTX pCtx, uint32_t *puHandle,
                                           uint32_t *puSeekMethod, uint64_t *poffSeek);
VBGLR3DECL(int) VbglR3GuestCtrlFileGetTell(PVBGLR3GUESTCTRLCMDCTX pCtx, uint32_t *puHandle);
VBGLR3DECL(int) VbglR3GuestCtrlFileGetSetSize(PVBGLR3GUESTCTRLCMDCTX pCtx, uint32_t *puHandle, uint64_t *pcbNew);

/* Guest -> Host. */
VBGLR3DECL(int) VbglR3GuestCtrlFileCbOpen(PVBGLR3GUESTCTRLCMDCTX pCtx, uint32_t uRc, uint32_t uFileHandle);
VBGLR3DECL(int) VbglR3GuestCtrlFileCbClose(PVBGLR3GUESTCTRLCMDCTX pCtx, uint32_t uRc);
VBGLR3DECL(int) VbglR3GuestCtrlFileCbError(PVBGLR3GUESTCTRLCMDCTX pCtx, uint32_t uRc);
VBGLR3DECL(int) VbglR3GuestCtrlFileCbRead(PVBGLR3GUESTCTRLCMDCTX pCtx, uint32_t uRc, void *pvData, uint32_t cbData);
VBGLR3DECL(int) VbglR3GuestCtrlFileCbReadOffset(PVBGLR3GUESTCTRLCMDCTX pCtx, uint32_t uRc,
                                                void *pvData, uint32_t cbData, int64_t offNew);
VBGLR3DECL(int) VbglR3GuestCtrlFileCbWrite(PVBGLR3GUESTCTRLCMDCTX pCtx, uint32_t uRc, uint32_t cbWritten);
VBGLR3DECL(int) VbglR3GuestCtrlFileCbWriteOffset(PVBGLR3GUESTCTRLCMDCTX pCtx, uint32_t uRc, uint32_t cbWritten, int64_t offNew);

VBGLR3DECL(int) VbglR3GuestCtrlFileCbSeek(PVBGLR3GUESTCTRLCMDCTX pCtx, uint32_t uRc, uint64_t offCurrent);
VBGLR3DECL(int) VbglR3GuestCtrlFileCbTell(PVBGLR3GUESTCTRLCMDCTX pCtx, uint32_t uRc, uint64_t offCurrent);
VBGLR3DECL(int) VbglR3GuestCtrlFileCbSetSize(PVBGLR3GUESTCTRLCMDCTX pCtx, uint32_t uRc, uint64_t cbNew);
VBGLR3DECL(int) VbglR3GuestCtrlProcCbStatus(PVBGLR3GUESTCTRLCMDCTX pCtx, uint32_t uPID, uint32_t uStatus, uint32_t fFlags,
                                            void *pvData, uint32_t cbData);
VBGLR3DECL(int) VbglR3GuestCtrlProcCbOutput(PVBGLR3GUESTCTRLCMDCTX pCtx, uint32_t uPID, uint32_t uHandle, uint32_t fFlags,
                                            void *pvData, uint32_t cbData);
VBGLR3DECL(int) VbglR3GuestCtrlProcCbStatusInput(PVBGLR3GUESTCTRLCMDCTX pCtx, uint32_t u32PID, uint32_t uStatus,
                                                 uint32_t fFlags, uint32_t cbWritten);

/** @}  */
# endif /* VBOX_WITH_GUEST_CONTROL defined */

/** @name Auto-logon handling
 * @{ */
VBGLR3DECL(int)     VbglR3AutoLogonReportStatus(VBoxGuestFacilityStatus enmStatus);
VBGLR3DECL(bool)    VbglR3AutoLogonIsRemoteSession(void);
/** @}  */

/** @name User credentials handling
 * @{ */
VBGLR3DECL(int)     VbglR3CredentialsQueryAvailability(void);
VBGLR3DECL(int)     VbglR3CredentialsRetrieve(char **ppszUser, char **ppszPassword, char **ppszDomain);
VBGLR3DECL(int)     VbglR3CredentialsRetrieveUtf16(PRTUTF16 *ppwszUser, PRTUTF16 *ppwszPassword, PRTUTF16 *ppwszDomain);
VBGLR3DECL(void)    VbglR3CredentialsDestroy(char *pszUser, char *pszPassword, char *pszDomain, uint32_t cPasses);
VBGLR3DECL(void)    VbglR3CredentialsDestroyUtf16(PRTUTF16 pwszUser, PRTUTF16 pwszPassword, PRTUTF16 pwszDomain,
                                                  uint32_t cPasses);
/** @}  */

/** @name CPU hotplug monitor
 * @{ */
VBGLR3DECL(int)     VbglR3CpuHotPlugInit(void);
VBGLR3DECL(int)     VbglR3CpuHotPlugTerm(void);
VBGLR3DECL(int)     VbglR3CpuHotPlugWaitForEvent(VMMDevCpuEventType *penmEventType, uint32_t *pidCpuCore, uint32_t *pidCpuPackage);
/** @} */

/** @name Page sharing
 * @{ */
struct VMMDEVSHAREDREGIONDESC;
VBGLR3DECL(int)     VbglR3RegisterSharedModule(char *pszModuleName, char *pszVersion, RTGCPTR64  GCBaseAddr, uint32_t cbModule,
                                               unsigned cRegions, struct VMMDEVSHAREDREGIONDESC *pRegions);
VBGLR3DECL(int)     VbglR3UnregisterSharedModule(char *pszModuleName, char *pszVersion, RTGCPTR64  GCBaseAddr, uint32_t cbModule);
VBGLR3DECL(int)     VbglR3CheckSharedModules(void);
VBGLR3DECL(bool)    VbglR3PageSharingIsEnabled(void);
VBGLR3DECL(int)     VbglR3PageIsShared(RTGCPTR pPage, bool *pfShared, uint64_t *puPageFlags);
/** @} */

# ifdef VBOX_WITH_DRAG_AND_DROP
/** @name Drag and Drop
 * @{ */
/**
 * Structure containing the context required for
 * either retrieving or sending a HGCM guest drag'n drop
 * commands from or to the host.
 *
 * Note: Do not change parameter order without also
 *       adapting all structure initializers.
 */
typedef struct VBGLR3GUESTDNDCMDCTX
{
    /** @todo This struct could be handy if we want to implement
     *        a second communication channel, e.g. via TCP/IP.
     *        Use a union for the HGCM stuff then. */

    /** HGCM client ID to use for communication. */
    uint32_t uClientID;
    /** The VM's current session ID. */
    uint64_t uSessionID;
    /** Protocol version to use. */
    uint32_t uProtocol;
    /** Number of parameters retrieved for the current command. */
    uint32_t uNumParms;
    /** Max chunk size (in bytes) for data transfers. */
    uint32_t cbMaxChunkSize;
} VBGLR3GUESTDNDCMDCTX, *PVBGLR3GUESTDNDCMDCTX;

/**
 * Enumeration for specifying the DnD meta data type.
 */
typedef enum VBGLR3GUESTDNDMETADATATYPE
{
    /** Unknown meta data type; don't use. */
    VBGLR3GUESTDNDMETADATATYPE_UNKNOWN = 0,
    /** Raw meta data; can be everything. */
    VBGLR3GUESTDNDMETADATATYPE_RAW,
    /** Meta data is an URI list, specifying objects. */
    VBGLR3GUESTDNDMETADATATYPE_URI_LIST,
    /** Blow the type up to 32-bit. */
    VBGLR3GUESTDNDMETADATATYPE_32BIT_HACK = 0x7fffffff
} VBGLR3GUESTDNDMETADATATYPE;

/**
 * Structure for keeping + handling DnD meta data.
 *
 * Note: Don't treat this struct as POD object, as the union has classes in it.
 */
typedef struct VBGLR3GUESTDNDMETADATA
{
    /** The meta data type the union contains. */
    VBGLR3GUESTDNDMETADATATYPE enmType;
    /** Pointer to actual meta data. */
    void    *pvMeta;
    /** Size (in bytes) of meta data. */
    uint32_t cbMeta;
} VBGLR3GUESTDNDMETADATA;

/** Pointer to VBGLR3GUESTDNDMETADATA. */
typedef VBGLR3GUESTDNDMETADATA *PVBGLR3GUESTDNDMETADATA;

/** Const pointer to VBGLR3GUESTDNDMETADATA. */
typedef const PVBGLR3GUESTDNDMETADATA CPVBGLR3GUESTDNDMETADATA;

/**
 * Enumeration specifying a DnD event type.
 */
typedef enum VBGLR3DNDEVENTTYPE
{
    VBGLR3DNDEVENTTYPE_INVALID        = 0,
    VBGLR3DNDEVENTTYPE_HG_ERROR       = 1,
    VBGLR3DNDEVENTTYPE_HG_ENTER       = 2,
    VBGLR3DNDEVENTTYPE_HG_MOVE        = 3,
    VBGLR3DNDEVENTTYPE_HG_LEAVE       = 4,
    VBGLR3DNDEVENTTYPE_HG_DROP        = 5,
    VBGLR3DNDEVENTTYPE_HG_RECEIVE     = 6,
    VBGLR3DNDEVENTTYPE_HG_CANCEL      = 7,
# ifdef VBOX_WITH_DRAG_AND_DROP_GH
    VBGLR3DNDEVENTTYPE_GH_ERROR       = 100,
    VBGLR3DNDEVENTTYPE_GH_REQ_PENDING = 101,
    VBGLR3DNDEVENTTYPE_GH_DROP        = 102,
# endif
    /** Blow the type up to 32-bit. */
    VBGLR3DNDEVENTTYPE_32BIT_HACK = 0x7fffffff
} VBGLR3DNDEVENTTYPE;

typedef struct VBGLR3DNDEVENT
{
    /** The event type the union contains. */
    VBGLR3DNDEVENTTYPE enmType;
    union
    {
        struct
        {
            /** Screen ID this request belongs to. */
            uint32_t uScreenID;
            /** Format list (UTF-8, \r\n separated). */
            char    *pszFormats;
            /** Size (in bytes) of pszFormats (\0 included). */
            uint32_t cbFormats;
            /** List of allowed DnD actions. */
            VBOXDNDACTIONLIST dndLstActionsAllowed;
        } HG_Enter;
        struct
        {
            /** Absolute X position of guest screen. */
            uint32_t uXpos;
            /** Absolute Y position of guest screen. */
            uint32_t uYpos;
            /** Default DnD action. */
            VBOXDNDACTION dndActionDefault;
        } HG_Move;
        struct
        {
            /** Absolute X position of guest screen. */
            uint32_t uXpos;
            /** Absolute Y position of guest screen. */
            uint32_t uYpos;
            /** Default DnD action. */
            VBOXDNDACTION dndActionDefault;
        } HG_Drop;
        struct
        {
            /** Meta data for the operation. */
            VBGLR3GUESTDNDMETADATA Meta;
        } HG_Received;
        struct
        {
            /** IPRT-style error code. */
            int rc;
        } HG_Error;
# ifdef VBOX_WITH_DRAG_AND_DROP_GH
        struct
        {
            /** Screen ID this request belongs to. */
            uint32_t uScreenID;
        } GH_IsPending;
        struct
        {
            /** Requested format by the host. */
            char    *pszFormat;
            /** Size (in bytes) of pszFormat (\0 included). */
            uint32_t cbFormat;
            /** Requested DnD action. */
            VBOXDNDACTION dndActionRequested;
        } GH_Drop;
# endif
    } u;
} VBGLR3DNDEVENT;
typedef VBGLR3DNDEVENT *PVBGLR3DNDEVENT;
typedef const PVBGLR3DNDEVENT CPVBGLR3DNDEVENT;

VBGLR3DECL(int)     VbglR3DnDConnect(PVBGLR3GUESTDNDCMDCTX pCtx);
VBGLR3DECL(int)     VbglR3DnDDisconnect(PVBGLR3GUESTDNDCMDCTX pCtx);

VBGLR3DECL(int)     VbglR3DnDEventGetNext(PVBGLR3GUESTDNDCMDCTX pCtx, PVBGLR3DNDEVENT *ppEvent);
VBGLR3DECL(void)    VbglR3DnDEventFree(PVBGLR3DNDEVENT pEvent);

VBGLR3DECL(int)     VbglR3DnDHGSendAckOp(PVBGLR3GUESTDNDCMDCTX pCtx, VBOXDNDACTION dndAction);
VBGLR3DECL(int)     VbglR3DnDHGSendReqData(PVBGLR3GUESTDNDCMDCTX pCtx, const char *pcszFormat);
VBGLR3DECL(int)     VbglR3DnDHGSendProgress(PVBGLR3GUESTDNDCMDCTX pCtx, uint32_t uStatus, uint8_t uPercent, int rcErr);
#  ifdef VBOX_WITH_DRAG_AND_DROP_GH
VBGLR3DECL(int)     VbglR3DnDGHSendAckPending(PVBGLR3GUESTDNDCMDCTX pCtx, VBOXDNDACTION dndActionDefault, VBOXDNDACTIONLIST dndLstActionsAllowed, const char* pcszFormats, uint32_t cbFormats);
VBGLR3DECL(int)     VbglR3DnDGHSendData(PVBGLR3GUESTDNDCMDCTX pCtx, const char *pszFormat, void *pvData, uint32_t cbData);
VBGLR3DECL(int)     VbglR3DnDGHSendError(PVBGLR3GUESTDNDCMDCTX pCtx, int rcOp);
#  endif /* VBOX_WITH_DRAG_AND_DROP_GH */
/** @} */
# endif /* VBOX_WITH_DRAG_AND_DROP */

/* Generic Host Channel Service. */
VBGLR3DECL(int)  VbglR3HostChannelInit(uint32_t *pidClient);
VBGLR3DECL(void) VbglR3HostChannelTerm(uint32_t idClient);
VBGLR3DECL(int)  VbglR3HostChannelAttach(uint32_t *pu32ChannelHandle, uint32_t u32HGCMClientId,
                                         const char *pszName, uint32_t u32Flags);
VBGLR3DECL(void) VbglR3HostChannelDetach(uint32_t u32ChannelHandle, uint32_t u32HGCMClientId);
VBGLR3DECL(int)  VbglR3HostChannelSend(uint32_t u32ChannelHandle, uint32_t u32HGCMClientId,
                                       void *pvData, uint32_t cbData);
VBGLR3DECL(int)  VbglR3HostChannelRecv(uint32_t u32ChannelHandle, uint32_t u32HGCMClientId,
                                       void *pvData, uint32_t cbData,
                                       uint32_t *pu32SizeReceived, uint32_t *pu32SizeRemaining);
VBGLR3DECL(int)  VbglR3HostChannelControl(uint32_t u32ChannelHandle, uint32_t u32HGCMClientId,
                                         uint32_t u32Code, void *pvParm, uint32_t cbParm,
                                         void *pvData, uint32_t cbData, uint32_t *pu32SizeDataReturned);
VBGLR3DECL(int)  VbglR3HostChannelEventWait(uint32_t *pu32ChannelHandle, uint32_t u32HGCMClientId,
                                            uint32_t *pu32EventId, void *pvParm, uint32_t cbParm,
                                            uint32_t *pu32SizeReturned);
VBGLR3DECL(int)  VbglR3HostChannelEventCancel(uint32_t u32ChannelHandle, uint32_t u32HGCMClientId);
VBGLR3DECL(int)  VbglR3HostChannelQuery(const char *pszName, uint32_t u32HGCMClientId, uint32_t u32Code,
                                        void *pvParm, uint32_t cbParm, void *pvData, uint32_t cbData,
                                        uint32_t *pu32SizeDataReturned);

/** @name Mode hint storage
 * @{ */
VBGLR3DECL(int) VbglR3ReadVideoMode(unsigned cDisplay, unsigned *cx,
                                    unsigned *cy, unsigned *cBPP, unsigned *x,
                                    unsigned *y, unsigned *fEnabled);
VBGLR3DECL(int) VbglR3WriteVideoMode(unsigned cDisplay, unsigned cx,
                                     unsigned cy, unsigned cBPP, unsigned x,
                                     unsigned y, unsigned fEnabled);
/** @} */

/** @name Generic HGCM
 * @{ */
VBGLR3DECL(int)     VbglR3HGCMConnect(const char *pszServiceName, HGCMCLIENTID *pidClient);
VBGLR3DECL(int)     VbglR3HGCMDisconnect(HGCMCLIENTID idClient);
struct VBGLIOCHGCMCALL;
VBGLR3DECL(int)     VbglR3HGCMCall(struct VBGLIOCHGCMCALL *pInfo, size_t cbInfo);
/** @} */

#endif /* IN_RING3 */
/** @} */

RT_C_DECLS_END

/** @} */

#endif /* !VBOX_INCLUDED_VBoxGuestLib_h */

