//	CMCIMixer.h
//
//	CMCIMixer class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

#define CMD_SOUNDTRACK_DONE						CONSTLIT("cmdSoundtrackDone")
#define CMD_SOUNDTRACK_NOW_PLAYING				CONSTLIT("cmdSoundtrackNowPlaying")
#define CMD_SOUNDTRACK_UPDATE_PLAY_POS			CONSTLIT("cmdSoundtrackUpdatePlayPos")

const DWORD CHECK_INTERVAL =					50;
const int FADE_LENGTH =							2000;
const int NORMAL_VOLUME =						1000;
const int FADE_DELAY =							25;

CMCIMixer::CMCIMixer (int iChannels) :
		m_hParent(NULL),
		m_iDefaultVolume(NORMAL_VOLUME),
		m_iCurChannel(-1),
		m_pNowPlaying(NULL),
		m_hProcessingThread(INVALID_HANDLE_VALUE),
		m_hWorkEvent(INVALID_HANDLE_VALUE),
		m_hResultEvent(INVALID_HANDLE_VALUE),
		m_hQuitEvent(INVALID_HANDLE_VALUE),
		m_bNoStopNotify(false),
		m_bDebugMode(false)

//	CMCIMixer constructor

	{
	//	Initialize events so we can post work even before our processing thread
	//	is running.

	m_hQuitEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hWorkEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hResultEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hAbortEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	}

CMCIMixer::~CMCIMixer (void)

//	CMCIMixer destructor

	{
	Shutdown();

	//	Free up our events

	::CloseHandle(m_hWorkEvent);
	::CloseHandle(m_hResultEvent);
	::CloseHandle(m_hQuitEvent);
	::CloseHandle(m_hAbortEvent);
	}

void CMCIMixer::AbortAllRequests (void)

//	AbortAllRequests
//
//	Remove all requests

	{
	CSmartLock Lock(m_cs);
	m_Request.DeleteAll();
	}

bool CMCIMixer::Boot (void)

//	Boot
//
//	Starts our processing thread running.

	{
	if (m_hProcessingThread == INVALID_HANDLE_VALUE)
		{
#ifdef DEBUG_SOUNDTRACK
		::kernelDebugLogPattern("[%x] Starting CMCIMixer.", ::GetCurrentThreadId());
#endif

		m_hProcessingThread = ::kernelCreateThread(ProcessingThread, this);
		}

	return true;
	}

bool CMCIMixer::CreateParentWindow (void)

//	CreateParentWindow
//
//	Creates the parent window

	{
	if (m_hParent == NULL)
		{
		WNDCLASSEX wc;

		//	Register window

		ZeroMemory(&wc, sizeof(wc));
		wc.cbSize = sizeof(wc);
		wc.style = CS_DBLCLKS;
		wc.lpfnWndProc = (WNDPROC)ParentWndProc;
		wc.lpszClassName = "TSUI_MCIParent";
		if (!::RegisterClassEx(&wc))
			return false;

		//	Create the window

		m_hParent = ::CreateWindowEx(0,	// WS_EX_TOPMOST,
				"TSUI_MCIParent", 
				"",
				WS_POPUP,
				0,
				0, 
				GetSystemMetrics(SM_CXSCREEN),
				GetSystemMetrics(SM_CYSCREEN), 
				NULL,
				NULL,
				NULL,
				this);
		if (m_hParent == NULL)
			return false;
		}

	return true;
	}

void CMCIMixer::EnqueueRequest (ERequestType iType, CMusicResource *pTrack, int iPos, bool bReplace)

//	EnqueueRequest
//
//	Enqueue a request

	{
	CSmartLock Lock(m_cs);
	int i;

	//	If we're replacing a request, the see if this request type is already in the queue

	if (bReplace)
		{
		for (i = 0; i < m_Request.GetCount(); i++)
			if (m_Request[i].iType == iType)
				{
				m_Request[i].pTrack = pTrack;
				m_Request[i].iPos = iPos;
				return;
				}
		}

	//	Add the request

	SRequest &NewRequest = m_Request.EnqueueAndOverwrite();
	NewRequest.iType = iType;
	NewRequest.pTrack = pTrack;
	NewRequest.iPos = iPos;

	::SetEvent(m_hWorkEvent);
	}

void CMCIMixer::FadeAtPos (int iPos)

//	FadeAtPos
//
//	Fades starting before the given position so that the fade is complete by the
//	time we reach iPos.

	{
	EnqueueRequest(typeWaitForPos, NULL, iPos - FADE_LENGTH);
	EnqueueRequest(typeFadeOut, NULL, iPos);
	EnqueueRequest(typeStop);
	}

void CMCIMixer::FadeNow (void)

//	FadeNow
//
//	Fades right now.

	{
	EnqueueRequest(typeFadeOut, NULL, Min(GetCurrentPlayPos() + FADE_LENGTH, GetCurrentPlayLength()));
	EnqueueRequest(typeStop);
	}

bool CMCIMixer::FindChannel (HWND hMCI, SChannel **retpChannel)

//	FindChannel
//
//	Finds the channel by child window

	{
	int i;

	for (i = 0; i < m_Channels.GetCount(); i++)
		if (m_Channels[i].hMCI == hMCI)
			{
			if (retpChannel)
				*retpChannel = &m_Channels[i];
			return true;
			}

	return false;
	}

int CMCIMixer::GetCurrentPlayLength (void)

//	GetCurrentPlayLength
//
//	Returns the length of the current track.
//
//	NOTE: These can only be called by other threads. It should never be called by
//	any processing functions because it will not return.

	{
	//	This is always stored in the channel and updated when we start playing.

	CSmartLock Lock(m_cs);
	if (m_iCurChannel == -1)
		return 0;

	return m_Channels[m_iCurChannel].iCurLength;
	}

int CMCIMixer::GetCurrentPlayPos (DWORD dwTimeout)

//	GetCurrentPlayPos
//
//	Returns the position of the current track.
//
//	NOTE: These can only be called by other threads. It should never be called by
//	any processing functions because it will not return.

	{
	//	Enqueue a request. We add TRUE to indicate that we should replace
	//	any existing GetPlayPos command.

	::ResetEvent(m_hResultEvent);
	EnqueueRequest(typeGetPlayPos, NULL, 0, true);

	//	Wait for result

	::WaitForSingleObject(m_hResultEvent, dwTimeout);

	//	Whether we succeed or not, get the result from the channel structure.

	CSmartLock Lock(m_cs);
	if (m_iCurChannel == -1)
		return 0;

	return m_Channels[m_iCurChannel].iCurPos;
	}

void CMCIMixer::GetDebugInfo (TArray<CString> *retLines) const

//	GetDebugInfo
//
//	Returns debug information about our current state

	{
	//	Add the current state of the processing thread

#ifdef DEBUG_SOUNDTRACK_STATE
	int i;
	m_cs.Lock();
	retLines->Insert(strPatternSubst(CONSTLIT("Mixer: %s"), GetRequestDesc(m_CurRequest)));
	for (i = 0; i < m_Request.GetCount(); i++)
		retLines->Insert(strPatternSubst(CONSTLIT("Queue[%02d]: %s"), i + 1, GetRequestDesc(m_Request.GetAt(i))));
	m_cs.Unlock();
#endif
	}

CString CMCIMixer::GetRequestDesc (const SRequest &Request) const

//	GetRequestDesc
//
//	Returns a description of a request

	{
	switch (Request.iType)
		{
		case typePlay:
			return strPatternSubst(CONSTLIT("ProcessPlay: %s"), Request.pTrack->GetFilespec());

		case typeStop:
			return CONSTLIT("ProcessStop");

		case typePlayPause:
			return CONSTLIT("ProcessPlayPause");

		case typeWaitForPos:
			return strPatternSubst(CONSTLIT("ProcessWaitForPos: %d"), Request.iPos);

		case typeFadeIn:
			return strPatternSubst(CONSTLIT("ProcessFadeIn: %s"), Request.pTrack->GetFilespec());

		case typeFadeOut:
			return CONSTLIT("ProcessFadeOut");

		case typeSetPaused:
		case typeSetUnpaused:
			return CONSTLIT("ProcessSetPlayPaused");

		case typeSetVolume:
			return CONSTLIT("ProcessSetVolume");

		case typeGetPlayPos:
			return CONSTLIT("typeGetPlayPos");

		default:
			return CONSTLIT("Idle");
		}
	}

bool CMCIMixer::InitChannels (void)

//	InitChannels
//
//	Initializes all channels

	{
	CSmartLock Lock(m_cs);

	int i;

	ASSERT(m_hParent == NULL);
	ASSERT(m_Channels.GetCount() == 0);

	if (!CreateParentWindow())
		return false;

	//	For now we only support 1 channel

	int iChannels = 1;

	//	Create all the MCI windows (one per channel).

	m_Channels.InsertEmpty(iChannels);
	for (i = 0; i < iChannels; i++)
		{
		m_Channels[i].hMCI = ::MCIWndCreate(m_hParent, 
				NULL,
				WS_OVERLAPPED | WS_CHILD | MCIWNDF_NOERRORDLG | MCIWNDF_NOMENU | MCIWNDF_NOPLAYBAR | MCIWNDF_NOTIFYALL,
				NULL);

		//	Abort if we can't do this

		if (m_Channels[i].hMCI == NULL)
			{
			::DestroyWindow(m_hParent);
			m_hParent = NULL;
			return false;
			}

		//	Initialize

		m_Channels[i].iState = stateNone;
		m_Channels[i].iCurPos = 0;
		m_Channels[i].iCurLength = 0;

		LogDebug(strPatternSubst(CONSTLIT("Created MCI window %x."), (size_t)m_Channels[i].hMCI));
		}

	m_iCurChannel = 0;

	return true;
	}

void CMCIMixer::LogDebug (const CString &sMsg)

//	LogDebug
//
//	Log debug info

	{
	if (!m_bDebugMode)
		return;

	::kernelDebugLogPattern("[%x] MCI DEBUG: %s", GetCurrentThreadId(), sMsg);
	}

void CMCIMixer::LogError (HWND hMCI, const CString &sState, const CString &sFilespec)

//	LogError
//
//	Log an error

	{
	CString sError;
	char *pDest = sError.GetWritePointer(1024);
	MCIWndGetError(hMCI, pDest, sError.GetLength());
	sError.Truncate(lstrlen(pDest));

	if (!sFilespec.IsBlank())
		::kernelDebugLogPattern("[%x] MCI ERROR %s [%s]: %s", GetCurrentThreadId(), sState, sFilespec, sError);
	else
		::kernelDebugLogPattern("[%x] MCI ERROR %s: %s", GetCurrentThreadId(), sState, sError);
	}

LONG CMCIMixer::OnNotifyMode (HWND hWnd, int iMode)

//	OnNotifyMode
//
//	Handle MCIWNDM_NOTIFYMODE

	{
	CSmartLock Lock(m_cs);

#ifdef DEBUG_SOUNDTRACK
	::kernelDebugLogPattern("[%x] OnNotifyMode[%x]: notify mode = %d.", ::GetCurrentThreadId(), (DWORD)hWnd, iMode);
#endif

	SChannel *pChannel;
	if (!FindChannel(hWnd, &pChannel))
		{
#ifdef DEBUG_SOUNDTRACK
		::kernelDebugLogPattern("OnNotifyMode: Unable to find channel for hWnd: %x.", (DWORD)hWnd);
#endif
		return 0;
		}

	switch (iMode)
		{
		case MCI_MODE_NOT_READY:
			break;

		case MCI_MODE_OPEN:
			break;

		case MCI_MODE_PAUSE:
			break;

		case MCI_MODE_PLAY:
			pChannel->iState = statePlaying;
			pChannel->iCurLength = GetPlayLength(pChannel->hMCI);
			pChannel->iCurPos = GetPlayPos(pChannel->hMCI);

			//	Notify that we're playing

			if (g_pHI)
				g_pHI->HIPostCommand(CMD_SOUNDTRACK_NOW_PLAYING, m_pNowPlaying);
			break;

		case MCI_MODE_RECORD:
			break;

		case MCI_MODE_SEEK:
			break;

		case MCI_MODE_STOP:
			pChannel->iState = stateStopped;
			pChannel->iCurLength = 0;
			pChannel->iCurPos = 0;

			//	Notify that we're done with this track

			if (g_pHI && !m_bNoStopNotify)
				g_pHI->HIPostCommand(CMD_SOUNDTRACK_DONE);
			break;
		}

	return 0;
	}

LONG CMCIMixer::OnNotifyPos (HWND hWnd, int iPos)

//	OnNotifyPos
//
//	Notify that we've played to a certain position.

	{
	CSmartLock Lock(m_cs);

	SChannel *pChannel;
	if (!FindChannel(hWnd, &pChannel))
		return 0;

	if (g_pHI)
		g_pHI->HIPostCommand(CMD_SOUNDTRACK_UPDATE_PLAY_POS, (void *)(size_t)iPos);

	return 0;
	}

LONG_PTR APIENTRY CMCIMixer::ParentWndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam)

//	ParentWndProc
//
//	Message handler

	{
	switch (message)
		{
		case WM_CREATE:
			{
			LPCREATESTRUCT pCreate = (LPCREATESTRUCT)(size_t)lParam;
			CMCIMixer *pThis = (CMCIMixer *)pCreate->lpCreateParams;
			::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);//64bit port WARNING - I have no idea whats up here
			return 0;
			}

		case MCIWNDM_NOTIFYMODE:
			{
			CMCIMixer *pThis = (CMCIMixer *)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
			return pThis->OnNotifyMode((HWND)(size_t)wParam, (int)lParam);
			}

		case MCIWNDM_NOTIFYPOS:
			{
			CMCIMixer *pThis = (CMCIMixer *)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
			return pThis->OnNotifyPos((HWND)(size_t)wParam, (int)lParam);
			}

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}

bool CMCIMixer::Play (CMusicResource *pTrack, int iPos)

//	Play
//
//	Stops all channels and begins playing the given track.

	{
	EnqueueRequest(typePlay, pTrack, iPos);
	return true;
	}

bool CMCIMixer::PlayFadeIn (CMusicResource *pTrack, int iPos)

//	Play
//
//	Stops all channels and begins playing the given track.

	{
	EnqueueRequest(typeFadeIn, pTrack, iPos);
	return true;
	}

void CMCIMixer::ProcessFadeIn (const SRequest &Request)

//	ProcessFadeIn
//
//	Fade in the given track starting at the given position.

	{
#ifdef DEBUG_SOUNDTRACK
	kernelDebugLogPattern("[%x] ProcessFadeIn", GetCurrentThreadId());
#endif

	//	Stop all channels

	ProcessStop(Request, true);

	//	Play on some channel
	if (m_iCurChannel < 0)
		return;
	HWND hMCI = m_Channels[m_iCurChannel].hMCI;

	//	Open new file

	CString sFilespec = Request.pTrack->GetFilespec();
	if (MCIWndOpen(hMCI, sFilespec.GetASCIIZPointer(), 0) != 0)
		{
		LogError(hMCI, CONSTLIT("ProcessFadeIn MCIWndOpen"), sFilespec);
		return;
		}

	LogDebug(strPatternSubst(CONSTLIT("MCIWndOpen %x %s."), (size_t)hMCI, sFilespec));

	//	Set state (we need to do this before we play because the callback inside
	//	MCIWndPlay needs m_pNowPlaying to be valid).

	m_pNowPlaying = Request.pTrack;

	//	Seek to the proper position

	MCIWndSeek(hMCI, Request.iPos);
	
	//	Play it

	MCIWndSetVolume(hMCI, 0);
	if (MCIWndPlay(hMCI) != 0)
		{
		LogError(hMCI, CONSTLIT("ProcessFadeIn MCIWndPlay"), sFilespec);
		return;
		}

	LogDebug(strPatternSubst(CONSTLIT("MCIWndPlay %x."), (size_t)hMCI));

	//	Fade in util we reach this position

	int iFullVolPos = Request.iPos + FADE_LENGTH;
	int iPrevPos = -1;
	while (true)
		{
		//	Wait a little bit

		if (!Wait(FADE_DELAY))
			return;

		//	How far into the fade

		int iCurPos = GetPlayPos(hMCI);
		UpdatePlayPos(m_iCurChannel, iCurPos);

		int iPlaying = iCurPos - Request.iPos;
		if (iPlaying <= 0 
				|| iPlaying >= FADE_LENGTH
				|| iCurPos == iPrevPos)
			{
			MCIWndSetVolume(hMCI, m_iDefaultVolume);
			return;
			}

		//	The volume should be proportional to how much we have left

		int iVolume = m_iDefaultVolume * iPlaying / FADE_LENGTH;
		MCIWndSetVolume(hMCI, iVolume);

		//	Remember our previous position to make sure we're making progress

		iPrevPos = iCurPos;
		}
	}

void CMCIMixer::ProcessFadeOut (const SRequest &Request)

//	ProcessFadeOut
//
//	Fade out

	{
#ifdef DEBUG_SOUNDTRACK
	kernelDebugLogPattern("[%x] ProcessFadeOut", GetCurrentThreadId());
#endif
	if (m_iCurChannel < 0)
		return;

	HWND hMCI = m_Channels[m_iCurChannel].hMCI;

	int iStartPos = GetPlayPos(hMCI);
	int iFadeLen = Request.iPos - iStartPos;
	if (iFadeLen <= 0)
		return;

	//	Loop until we've reached out desired position (or until the end of the
	//	track).

	int iPrevPos = -1;
	while (true)
		{
		//	Wait a little bit

		if (!Wait(FADE_DELAY))
			return;

		//	How much longer until we fade completely?

		int iCurPos = GetPlayPos(hMCI);
		UpdatePlayPos(m_iCurChannel, iCurPos);

		int iLeft = Request.iPos - iCurPos;
		if (iLeft <= 0 
				|| iCurPos == iPrevPos)
			{
			MCIWndSetVolume(hMCI, 0);
			return;
			}

		//	The volume should be proportional to how much we have left

		int iVolume = m_iDefaultVolume * iLeft / iFadeLen;
		MCIWndSetVolume(hMCI, iVolume);

		//	Remember our previous position to make sure we're making progress

		iPrevPos = iCurPos;
		}
	}

void CMCIMixer::ProcessPlay (const SRequest &Request)

//	ProcessPlay
//
//	Play the file

	{
#ifdef DEBUG_SOUNDTRACK
	kernelDebugLogPattern("[%x] ProcessPlay: %s", GetCurrentThreadId(), Request.pTrack->GetFilespec());
#endif

	//	Stop all channels

	ProcessStop(Request, true);

	//	Play on some channel

	if (m_iCurChannel < 0)
		return;
	HWND hMCI = m_Channels[m_iCurChannel].hMCI;

	//	Open new file

	CString sFilespec = Request.pTrack->GetFilespec();
	if (MCIWndOpen(hMCI, sFilespec.GetASCIIZPointer(), 0) != 0)
		{
		LogError(hMCI, CONSTLIT("ProcessPlay MCIWndOpen"), sFilespec);
		return;
		}

	LogDebug(strPatternSubst(CONSTLIT("MCIWndOpen %x %s."), (size_t)hMCI, sFilespec));

	//	Set state (we need to do this before we play because the callback inside
	//	MCIWndPlay needs m_pNowPlaying to be valid).

	m_pNowPlaying = Request.pTrack;

	//	Seek to the proper position

	MCIWndSeek(hMCI, Request.iPos);
	UpdatePlayPos(m_iCurChannel, Request.iPos);
	
	//	Play it

	MCIWndSetVolume(hMCI, m_iDefaultVolume);
	if (MCIWndPlay(hMCI) != 0)
		{
		LogError(hMCI, CONSTLIT("ProcessPlay MCIWndPlay"), sFilespec);
		return;
		}

	LogDebug(strPatternSubst(CONSTLIT("MCIWndPlay %x."), (size_t)hMCI));

#ifdef DEBUG_SOUNDTRACK
	kernelDebugLogPattern("[%x] ProcessPlay done", GetCurrentThreadId());
#endif
	}

void CMCIMixer::ProcessPlayPause (const SRequest &Request)

//	ProcessPlayPause
//
//	Pause and play current track.

	{
#ifdef DEBUG_SOUNDTRACK
	kernelDebugLogPattern("[%x] ProcessPlayPause", GetCurrentThreadId());
#endif

	if (m_iCurChannel < 0)
		return;
	HWND hMCI = m_Channels[m_iCurChannel].hMCI;

	int iMode = MCIWndGetMode(hMCI, 0, NULL);
	if (iMode == MCI_MODE_PLAY)
		MCIWndPause(hMCI);
	else if (iMode == MCI_MODE_PAUSE)
		MCIWndResume(hMCI);
	}

bool CMCIMixer::ProcessRequest (void)

//	ProcessRequest
//
//	Process a single request off the queue. If there are no more requests, then
//	we reset the worker event.

	{
	bool bMoreEvents;
	SRequest Request;

	//	Lock while we pull something off the queue

	m_cs.Lock();
	if (m_Request.GetCount() > 0)
		{
		Request = m_Request.Head();
		m_Request.Dequeue();
		}
	else
		Request.iType = typeNone;

	//	If our queue is empty, then reset the work event

	if (m_Request.GetCount() == 0)
		{
		::ResetEvent(m_hWorkEvent);
		bMoreEvents = false;
		}
	else
		bMoreEvents = true;

#ifdef DEBUG_SOUNDTRACK_STATE
	m_CurRequest = Request;
#endif

	//	Unlock so we can process the event

	m_cs.Unlock();

	//	Now process the event based on the type. We do this inside a try/catch
	//	in case there are system problems with the music

	try
		{
		switch (Request.iType)
			{
			case typeFadeIn:
				ProcessFadeIn(Request);
				break;

			case typeFadeOut:
				ProcessFadeOut(Request);
				break;

			case typeGetPlayPos:
				{
				int iValue = GetPlayPos(m_Channels[m_iCurChannel].hMCI);
				UpdatePlayPos(m_iCurChannel, iValue);
				::SetEvent(m_hResultEvent);
				break;
				}

			case typePlay:
				ProcessPlay(Request);
				break;

			case typePlayPause:
				ProcessPlayPause(Request);
				break;

			case typeSetVolume:
				ProcessSetVolume(Request);
				break;

			case typeStop:
#ifdef DEBUG_SOUNDTRACK
				kernelDebugLogPattern("[%x] ProcessStop requested.", GetCurrentThreadId());

#endif
				ProcessStop(Request);
				break;

			case typeWaitForPos:
				ProcessWaitForPos(Request);
				break;

			case typeSetPaused:
			case typeSetUnpaused:
				ProcessSetPlayPaused(Request);
				break;
			}
		}
	catch (...)
		{
		::kernelDebugLogPattern("Crash in CMCIMixer::ProcessRequest.");
		}

#ifdef DEBUG_SOUNDTRACK_STATE
	m_cs.Lock();
	m_CurRequest.iType = typeNone;
	m_cs.Unlock();
#endif

	//	Return whether we think there are more events in the queue. This is a
	//	heuristic, because someone might have changed the queue from under us,
	//	but callers just use this as a hint. No harm if we're wrong.

	return bMoreEvents;
	}

void CMCIMixer::ProcessSetPlayPaused (const SRequest &Request)

//	ProcessSetPlayPaused
//
//	Set to play or paused

	{
	if (m_iCurChannel < 0)
		return;
	HWND hMCI = m_Channels[m_iCurChannel].hMCI;

	int iMode = MCIWndGetMode(hMCI, 0, NULL);

	if (Request.iType == typeSetPaused)
		{
		if (iMode == MCI_MODE_PLAY)
			{
#ifdef DEBUG_SOUNDTRACK
			kernelDebugLogPattern("[%x] ProcessSetPlayPaused: Pause", GetCurrentThreadId());
#endif
			MCIWndPause(hMCI);
			}
		}
	else if (Request.iType == typeSetUnpaused)
		{
		if (iMode == MCI_MODE_PAUSE)
			{
#ifdef DEBUG_SOUNDTRACK
			kernelDebugLogPattern("[%x] ProcessSetPlayPaused: Resume", GetCurrentThreadId());
#endif
			MCIWndResume(hMCI);
			}
		}
	}

void CMCIMixer::ProcessSetVolume (const SRequest &Request)

//	ProcessSetVolume
//
//	Sets the volume

	{
	m_iDefaultVolume = Max(0, Request.iPos);

	if (m_iCurChannel < 0)
		return;
	HWND hMCI = m_Channels[m_iCurChannel].hMCI;
	MCIWndSetVolume(hMCI, m_iDefaultVolume);
	LogDebug(strPatternSubst(CONSTLIT("MCIWndSetVolume %x."), (size_t)hMCI));
	}

void CMCIMixer::ProcessStop (const SRequest &Request, bool bNoNotify)

//	ProcessStop
//
//	Stop playing

	{
	int i;

	bool bOldNotify = m_bNoStopNotify;
	if (bNoNotify)
		m_bNoStopNotify = true;

	for (i = 0; i < m_Channels.GetCount(); i++)
		if (m_Channels[i].iState == statePlaying || m_Channels[i].iState == stateStopped)
			{
#ifdef DEBUG_SOUNDTRACK
			kernelDebugLogPattern("[%x] ProcessStop", GetCurrentThreadId());
#endif

			if (m_Channels[i].iState != stateStopped)
				{
				MCIWndStop(m_Channels[i].hMCI);
				LogDebug(strPatternSubst(CONSTLIT("MCIWndStop %x."), (size_t)m_Channels[i].hMCI));
				}

			MCIWndClose(m_Channels[i].hMCI);
			UpdatePlayPos(i, 0);
			LogDebug(strPatternSubst(CONSTLIT("MCIWndClose %x."), (size_t)m_Channels[i].hMCI));

			m_Channels[i].iState = stateNone;
			}

	m_bNoStopNotify = bOldNotify;
	m_pNowPlaying = NULL;
	}

void CMCIMixer::ProcessWaitForPos (const SRequest &Request)

//	ProcessWaitForPos
//
//	Waits until the current playback position is at the given position.

	{
#ifdef DEBUG_SOUNDTRACK
	kernelDebugLogPattern("[%x] ProcessWaitForPos", GetCurrentThreadId());
#endif

	if (m_iCurChannel < 0)
		return;
	HWND hMCI = m_Channels[m_iCurChannel].hMCI;

	int iPrevPos = -1;
	while (true)
		{
		//	Keep looping until we reach the given position or until we ask asked
		//	to stop.
		//
		//	We must be in play mode.

		int iMode = MCIWndGetMode(hMCI, 0, NULL);
		if (iMode != MCI_MODE_PLAY)
			return;

		//	Get our current position. If we reached the position, then we're done.

		int iCurPos = GetPlayPos(hMCI);
		UpdatePlayPos(m_iCurChannel, iCurPos);

		if (iCurPos >= Request.iPos
				|| iCurPos == iPrevPos)
			return;

		//	Otherwise, we wait for a little bit (and we check to see if someone
		//	wants us to quit.

		if (!Wait(CHECK_INTERVAL))
			return;

		//	Loop and check again, but remember our current position 
		//	to make sure we're making progress

		iPrevPos = iCurPos;
		}
	}

DWORD WINAPI CMCIMixer::ProcessingThread (LPVOID pData)

//	ProcessingThread
//
//	Processing thread

	{
	CMCIMixer *pThis = (CMCIMixer *)pData;

	//	Create the parent window that will receive notifications.
	//	We do this in the background thread because this is the only thread that
	//	will send messages to the MCI window.
	//	
	//	NOTE: If we could not create the window then something is wrong and we
	//	disable everything.

	pThis->InitChannels();

	//	Loop until we're done

	while (true)
		{
		HANDLE Handles[2];
		Handles[0] = pThis->m_hQuitEvent;
		const DWORD WAIT_QUIT_EVENT = (WAIT_OBJECT_0);
		Handles[1] = pThis->m_hWorkEvent;
		const DWORD WAIT_WORK_EVENT = (WAIT_OBJECT_0 + 1);
		const DWORD WAIT_MESSAGES = (WAIT_OBJECT_0 + 2);

		//	Wait for work to do

		DWORD dwWait = ::MsgWaitForMultipleObjects(2, Handles, FALSE, INFINITE, QS_ALLINPUT);

		//	Do the work

		if (dwWait == WAIT_QUIT_EVENT)
			return 0;
		else if (dwWait == WAIT_WORK_EVENT)
			{
			//	Process requests from the queue until we're done. NOTE: This
			//	will also reset the work event.

			while (pThis->ProcessRequest())
				{
				//	Quit, if necessary

				if (::WaitForSingleObject(pThis->m_hQuitEvent, 0) == WAIT_OBJECT_0)
					return 0;
				}

			//	Keep looping
			}
		else if (dwWait == WAIT_MESSAGES)
			{
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				{
		        TranslateMessage(&msg);
		        DispatchMessage(&msg);
				}
			}
		}

	return 0;
	}

void CMCIMixer::SetPlayPaused (bool bPlay)

//	SetPlayPaused
//
//	Pause/Play

	{
	EnqueueRequest(bPlay ? typeSetUnpaused : typeSetPaused);
	}

void CMCIMixer::SetVolume (int iVolume)

//	SetVolume
//
//	Sets the volume, where 1000 is normal volume and 0 is no volume.

	{
	EnqueueRequest(typeSetVolume, NULL, iVolume);
	}

void CMCIMixer::Shutdown (void)

//	Shutdown
//
//	Program is quitting, so clean up. NOTE: This is the opposite of Boot, so we
//	don't clean up our events in case Boot gets called again.

	{
	if (m_hProcessingThread != INVALID_HANDLE_VALUE)
		{
		::SetEvent(m_hQuitEvent);
		::kernelDispatchUntilEventSet(m_hProcessingThread, 5000);

		::CloseHandle(m_hProcessingThread);
		m_hProcessingThread = INVALID_HANDLE_VALUE;
		}

	//	Now that the processing thread has stopped, we can clean up all this
	//	without locking.

	if (m_hParent)
		{
		::DestroyWindow(m_hParent);
		m_hParent = NULL;
		}

	m_Channels.DeleteAll();
	m_Request.DeleteAll();
	m_iCurChannel = -1;
	m_pNowPlaying = NULL;
	}

void CMCIMixer::Stop (void)

//	Stop
//
//	Stops playing all.

	{
	EnqueueRequest(typeStop);
	}

void CMCIMixer::TogglePausePlay (void)

//	TogglePausePlay
//
//	Pause/Play

	{
	EnqueueRequest(typePlayPause);
	}

void CMCIMixer::UpdatePlayPos (int iChannel, int iPos)

//	UpdatePlayPos
//
//	Updates the play position

	{
	//	If iPos is -1 then we get the position from the MCI object. This should
	//	only be called from inside the processing thread.

	if (iPos == -1)
		iPos = GetPlayPos(m_Channels[iChannel].hMCI);

	//	Update the position

	CSmartLock Lock(m_cs);
	m_Channels[iChannel].iCurPos = iPos;
	}

bool CMCIMixer::Wait (DWORD dwTimeout)

//	Wait
//
//	Waits until the timeout or until we are asked to quit or abort. Returns 
//	FALSE we we need to quit or abort.

	{
	HANDLE Handles[2];
	Handles[0] = m_hQuitEvent;
	const DWORD WAIT_QUIT_EVENT = (WAIT_OBJECT_0);
	Handles[1] = m_hAbortEvent;
	const DWORD WAIT_ABORT_EVENT = (WAIT_OBJECT_0 + 1);

	DWORD dwWait = ::WaitForMultipleObjects(2, Handles, FALSE, dwTimeout);

	//	Quit or abort?

	if (dwWait == WAIT_QUIT_EVENT
			|| dwWait == WAIT_ABORT_EVENT)
		return false;

	//	Done

	return true;
	}
