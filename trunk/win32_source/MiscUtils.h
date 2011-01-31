/*
 * Copyright 2010 by Seth N. Hetu
 *
 * Please refer to the end of the file for licensing information
 */


//Taking more code out of MainFile, since it's logically unrelated

/**
 * Check if we are running windows Vista, 7, or anything with UAC
 */
bool IsVistaOrMore()
{
	OSVERSIONINFO OSversion;
	OSversion.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
	GetVersionEx(&OSversion);
	return (OSversion.dwMajorVersion>=6);
}


/**
 * Elevate and run a new instance of WaitZar
 */
bool elevateWaitZar(LPCWSTR wzFileName)
{
	//Define our task
	SHELLEXECUTEINFO wzInfo;
    wzInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	wzInfo.fMask = 0;
	wzInfo.hwnd = NULL;
	wzInfo.lpVerb = L"runas";
	wzInfo.lpFile = wzFileName;
	wzInfo.lpParameters = L"runasadmin"; //Is this necessary?
    wzInfo.lpDirectory = NULL;
    wzInfo.nShow = SW_NORMAL;

	//Start the task
	return (ShellExecuteEx(&wzInfo) == TRUE);
}




/**
 * Borrowed from KeyMagic.
 *
 * Note to self: I've tried many times to re-write this function and make it less messy.
 *  The problem is, the code is pretty good (checks all error conditions, handles
 *  the various intricate options of Windows access control settings, etc.), and I
 *  don't think I could clean it up without breaking some of this. Even though the
 *  coding style is vastly different from my own, I think I'll have to just accept
 *  the clutter for the time being --it serves its purpose, and I have more important
 *  code to fix.
 */
bool IsAdmin()
{
   BOOL   fReturn         = FALSE;
   DWORD  dwStatus;
   DWORD  dwAccessMask;
   DWORD  dwAccessDesired;
   DWORD  dwACLSize;
   DWORD  dwStructureSize = sizeof(PRIVILEGE_SET);
   PACL   pACL            = NULL;
   PSID   psidAdmin       = NULL;

   HANDLE hToken              = NULL;
   HANDLE hImpersonationToken = NULL;

   PRIVILEGE_SET   ps;
   GENERIC_MAPPING GenericMapping;

   PSECURITY_DESCRIPTOR     psdAdmin           = NULL;
   SID_IDENTIFIER_AUTHORITY SystemSidAuthority = {SECURITY_NT_AUTHORITY};


   /*
      Determine if the current thread is running as a user that is a member of
      the local admins group.  To do this, create a security descriptor that
      has a DACL which has an ACE that allows only local aministrators access.
      Then, call AccessCheck with the current thread's token and the security
      descriptor.  It will say whether the user could access an object if it
      had that security descriptor.  Note: you do not need to actually create
      the object.  Just checking access against the security descriptor alone
      will be sufficient.
   */
   const DWORD ACCESS_READ  = 1;
   const DWORD ACCESS_WRITE = 2;


   //Perform the following once, but allow breaking out at any time.
   // "Break" will lead immediately to the clean-up tasks. 
   // This is done to remove the __try/__leave/__finally code (which is a nasty Win32 leftover
   //    that even Microsoft seems to advise against in general).
   do {
      /*
         AccessCheck() requires an impersonation token.  We first get a primary
         token and then create a duplicate impersonation token.  The
         impersonation token is not actually assigned to the thread, but is
         used in the call to AccessCheck.  Thus, this function itself never
         impersonates, but does use the identity of the thread.  If the thread
         was impersonating already, this function uses that impersonation context.
      */
      if (!OpenThreadToken(GetCurrentThread(), TOKEN_DUPLICATE|TOKEN_QUERY,
		  TRUE, &hToken))
      {
         if (GetLastError() != ERROR_NO_TOKEN)
            break;

         if (!OpenProcessToken(GetCurrentProcess(),
			 TOKEN_DUPLICATE|TOKEN_QUERY, &hToken))
            break;
      }

      if (!DuplicateToken (hToken, SecurityImpersonation,
		  &hImpersonationToken))
		  break;


      /*
        Create the binary representation of the well-known SID that
        represents the local administrators group.  Then create the security
        descriptor and DACL with an ACE that allows only local admins access.
        After that, perform the access check.  This will determine whether
        the current user is a local admin.
      */
      if (!AllocateAndInitializeSid(&SystemSidAuthority, 2,
                                    SECURITY_BUILTIN_DOMAIN_RID,
                                    DOMAIN_ALIAS_RID_ADMINS,
                                    0, 0, 0, 0, 0, 0, &psidAdmin))
         break;

      psdAdmin = (SECURITY_DESCRIPTOR*)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
      if (psdAdmin == NULL)
         break;

      if (!InitializeSecurityDescriptor(psdAdmin,
		  SECURITY_DESCRIPTOR_REVISION))
         break;

      // Compute size needed for the ACL.
      dwACLSize = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) +
                  GetLengthSid(psidAdmin) - sizeof(DWORD);

      pACL = (PACL)LocalAlloc(LPTR, dwACLSize);
      if (pACL == NULL)
         break;

      if (!InitializeAcl(pACL, dwACLSize, ACL_REVISION2))
         break;

      dwAccessMask= ACCESS_READ | ACCESS_WRITE;

      if (!AddAccessAllowedAce(pACL, ACL_REVISION2, dwAccessMask,
		  psidAdmin))
         break;

      if (!SetSecurityDescriptorDacl(psdAdmin, TRUE, pACL, FALSE))
         break;

      /*
         AccessCheck validates a security descriptor somewhat; set the group
         and owner so that enough of the security descriptor is filled out to
         make AccessCheck happy.
      */
      SetSecurityDescriptorGroup(psdAdmin, psidAdmin, FALSE);
      SetSecurityDescriptorOwner(psdAdmin, psidAdmin, FALSE);

      if (!IsValidSecurityDescriptor(psdAdmin))
         break;

      dwAccessDesired = ACCESS_READ;

      /*
         Initialize GenericMapping structure even though you
         do not use generic rights.
      */
      GenericMapping.GenericRead    = ACCESS_READ;
      GenericMapping.GenericWrite   = ACCESS_WRITE;
      GenericMapping.GenericExecute = 0;
      GenericMapping.GenericAll     = ACCESS_READ | ACCESS_WRITE;

      if (!AccessCheck(psdAdmin, hImpersonationToken, dwAccessDesired,
                       &GenericMapping, &ps, &dwStructureSize, &dwStatus,
                       &fReturn))
      {
         fReturn = FALSE;
         break;
      }

   } while (false);

    // Clean up.
    if (pACL) LocalFree(pACL);
    if (psdAdmin) LocalFree(psdAdmin);
    if (psidAdmin) FreeSid(psidAdmin);
    if (hImpersonationToken) CloseHandle (hImpersonationToken);
    if (hToken) CloseHandle (hToken);

   return (fReturn==TRUE);
}



/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
