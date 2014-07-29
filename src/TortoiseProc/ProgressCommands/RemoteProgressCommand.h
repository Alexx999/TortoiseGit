// TortoiseGit - a Windows shell extension for easy version control

// Copyright (C) 2013-2014 - TortoiseGit

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

#include "stdafx.h"
#include "GitProgressList.h"
#include "../TortoiseShell/resource.h"
#include "AppUtils.h"

class RemoteProgressCommand : public ProgressCommand
{
protected:
	CTGitPath m_url;
	CString m_RefSpec;
	CString m_remote;

	static int RemoteProgressCallback(const char* str, int len, void* data)
	{
		((CGitProgressList*)data)->SetProgressLabelText(CUnicodeUtils::GetUnicode(CStringA(str, len)));
		return 0;
	}
	static int RemoteCompletionCallback(git_remote_completion_type /*type*/, void* /*data*/)
	{
		// this method is unused by libgit2 so far
		// TODO: "m_pAnimate->Stop();" and "m_pAnimate->ShowWindow(SW_HIDE);", cleanup possible in GitProgressList::Notify
		return 0;
	}
	static int RemoteUpdatetipsCallback(const char* refname, const git_oid* oldOid, const git_oid* newOid, void* data)
	{
		((CGitProgressList*)data)->AddNotify(new RefUpdateNotificationData(refname, oldOid, newOid));
		return 0;
	}

	class RefUpdateNotificationData : public CGitProgressList::NotificationData
	{
	public:
		RefUpdateNotificationData(const char* refname, const git_oid* oldOid, const git_oid* newOid)
		: NotificationData()
		{
			CString str = CUnicodeUtils::GetUnicode(refname);
			m_NewHash = newOid->id;
			m_OldHash = oldOid->id;
			sActionColumnText.LoadString(IDS_GITACTION_UPDATE_REF);
			sPathColumnText.Format(_T("%s\t %s -> %s"), str, m_OldHash.ToString().Left(g_Git.GetShortHASHLength()), m_NewHash.ToString().Left(g_Git.GetShortHASHLength()));
		};
		virtual void GetContextMenu(CIconMenu& popup, CGitProgressList::ContextMenuActionList& actions)
		{
			actions.push_back([&]()
			{
				CString cmd = _T("/command:log");
				cmd += _T(" /path:\"") + g_Git.m_CurrentDir + _T("\"");
				if (!m_OldHash.IsEmpty())
					cmd += _T(" /startrev:") + m_OldHash.ToString();
				if (!m_NewHash.IsEmpty())
					cmd += _T(" /endrev:") + m_NewHash.ToString();
				CAppUtils::RunTortoiseGitProc(cmd);
			});
			popup.AppendMenuIcon(actions.size(), IDS_MENULOG, IDI_LOG);
		};
	protected:
		CGitHash	m_OldHash;
		CGitHash	m_NewHash;
	};

public:
	void SetUrl(const CString& url) { m_url.SetFromUnknown(url); }
	void SetRefSpec(CString spec){ m_RefSpec = spec; }
	void SetRemote(const CString& remote) { m_remote = remote; }
};

class CSmartAnimation
{
	CAnimateCtrl* m_pAnimate;

public:
	CSmartAnimation(CAnimateCtrl* pAnimate)
	{
		m_pAnimate = pAnimate;
		if (m_pAnimate)
		{
			m_pAnimate->ShowWindow(SW_SHOW);
			m_pAnimate->Play(0, INT_MAX, INT_MAX);
		}
	}
	~CSmartAnimation()
	{
		if (m_pAnimate)
		{
			m_pAnimate->Stop();
			m_pAnimate->ShowWindow(SW_HIDE);
		}
	}
};
