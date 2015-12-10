/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/
typedef HRESULT (STDAPICALLTYPE* FN_DLLGETCLASSOBJECT)(REFCLSID clsid, REFIID iid, void** ppv);

HRESULT CreateObjectFromPath(TCHAR* pPath, REFCLSID clsid, IUnknown** ppUnk)
{
	// load the target DLL directly
	HMODULE lib = LoadLibrary(pPath);//see https://github.com/fancycode/MemoryModule
	if (!lib)
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	// the entry point is an exported function
	FN_DLLGETCLASSOBJECT fn = (FN_DLLGETCLASSOBJECT)GetProcAddress(lib, "DllGetClassObject");
	if (fn == NULL)
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	// create a class factory
	IUnknown* pUnk;
	HRESULT hr = fn(clsid,  IID_IUnknown,  (void**)(IUnknown**)&pUnk);
	if (SUCCEEDED(hr))
	{
		IClassFactory* pCF = (IClassFactory*)pUnk;
		if (pCF == NULL)
		{
			hr = E_NOINTERFACE;
		}
		else
		{
			// ask the class factory to create the object
			hr = pCF->CreateInstance(NULL, IID_IUnknown, (void**)ppUnk);
		}
	}

	return hr;
}
/*
IUnknownPtr pUnk;
HRESULT hr = CreateObjectFromPath(TEXT("c:\\path\\to\\myfilter.dll"), IID_MyFilter, &pUnk);
if (SUCCEEDED(hr))
{
	IBaseFilterPtr pFilter = pUnk;
	pGraph->AddFilter(pFilter, L"Private Filter");
	pGraph->RenderFile(pMediaClip, NULL);
}*/
IPin *GetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, const GUID* guid=NULL)
{
    bool       bFound = false;
    IEnumPins  *pEnum;
    IPin       *pPin;

    pFilter->EnumPins(&pEnum);
    while(pEnum->Next(1, &pPin, 0) == S_OK)
    {
        PIN_DIRECTION PinDirThis;
        pPin->QueryDirection(&PinDirThis);

        if(PinDir == PinDirThis)
        {
            if(guid == NULL)
            {
                bFound = true;
            }
            else
            {
                AM_MEDIA_TYPE mediaType;
                HRESULT hr = pPin->ConnectionMediaType(&mediaType);
                bFound = SUCCEEDED(hr) && IsEqualGUID(mediaType.majortype, *guid);
            }
        }
        //LPWSTR id;
        //pPin->QueryId(&id);
        //CoTaskMemFree(id);

        if (bFound)
            break;
        pPin->Release();
    }
    pEnum->Release();
    return (bFound ? pPin : 0);
}
namespace DirectShowHelpers
{
    bool checkDShowAvailability()
    {
        ComSmartPtr <IGraphBuilder> graph;
        return SUCCEEDED (graph.CoCreateInstance (CLSID_FilterGraph));
    }

    //======================================================================
    class VideoRenderer
    {
    public:
        VideoRenderer() {}
        virtual ~VideoRenderer() {}

        virtual HRESULT create (ComSmartPtr <IGraphBuilder>& graphBuilder,
                                ComSmartPtr <IBaseFilter>& baseFilter, HWND hwnd, String& err) = 0;

        virtual void setVideoWindow (HWND hwnd) = 0;
        virtual void setVideoPosition (HWND hwnd, long videoWidth, long videoHeight) = 0;
        virtual void repaintVideo (HWND hwnd, HDC hdc) = 0;
        virtual void displayModeChanged() = 0;
        virtual HRESULT getVideoSize (long& videoWidth, long& videoHeight) = 0;
    };

    //======================================================================
    class VMR7  : public VideoRenderer
    {
    public:
        VMR7() {}

        HRESULT create (ComSmartPtr <IGraphBuilder>& graphBuilder,
                        ComSmartPtr <IBaseFilter>& baseFilter, HWND hwnd, String& err)
        {
            ComSmartPtr <IVMRFilterConfig> filterConfig;

            err = "CLSID_VideoMixingRenderer";
            HRESULT hr = baseFilter.CoCreateInstance (CLSID_VideoMixingRenderer);


            if (SUCCEEDED (hr))
            {
                err = "AddFilter VMR-7";
                hr = graphBuilder->AddFilter (baseFilter, L"VMR-7");
            }
            else{ return hr;}
            if (SUCCEEDED (hr))
            {
                err = "QueryInterface filterConfig";
                hr = baseFilter.QueryInterface (filterConfig);
            }
            else{ return hr;}
            if (SUCCEEDED (hr))
            {
                err = "SetRenderingMode VMRMode_Windowless";
                hr = filterConfig->SetRenderingMode (VMRMode_Windowless);
            }
            else{ return hr;}
            if (SUCCEEDED (hr))
            {
                err = "QueryInterface windowlessControl";
                hr = baseFilter.QueryInterface (windowlessControl);
            }
            else{ return hr;}
            if (SUCCEEDED (hr))
            {
                err = "windowlessControl SetVideoClippingWindow";
                hr = windowlessControl->SetVideoClippingWindow (hwnd);
            }
            else{ return hr;}
            if (SUCCEEDED (hr))
            {
                err = "windowlessControl SetAspectRatioMode";
                hr = windowlessControl->SetAspectRatioMode (VMR_ARMODE_LETTER_BOX);
            }
            else{ return hr;}

            return hr;
        }

        void setVideoWindow (HWND hwnd)
        {
            windowlessControl->SetVideoClippingWindow (hwnd);
        }

        void setVideoPosition (HWND hwnd, long videoWidth, long videoHeight)
        {
            RECT src, dest;

            SetRect (&src, 0, 0, videoWidth, videoHeight);
            GetClientRect (hwnd, &dest);

            windowlessControl->SetVideoPosition (&src, &dest);
        }

        void repaintVideo (HWND hwnd, HDC hdc)
        {
            windowlessControl->RepaintVideo (hwnd, hdc);
        }

        void displayModeChanged()
        {
            windowlessControl->DisplayModeChanged();
        }

        HRESULT getVideoSize (long& videoWidth, long& videoHeight)
        {
            return windowlessControl->GetNativeVideoSize (&videoWidth, &videoHeight, nullptr, nullptr);
        }

    private:
        ComSmartPtr <IVMRWindowlessControl> windowlessControl;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VMR7)
    };


    //======================================================================
#if JUCE_MEDIAFOUNDATION
    class EVR : public VideoRenderer
    {
    public:
        EVR() {}

        HRESULT create (ComSmartPtr <IGraphBuilder>& graphBuilder,
                        ComSmartPtr <IBaseFilter>& baseFilter, HWND hwnd, String& err)
        {
            ComSmartPtr <IMFGetService> getService;

            HRESULT hr = baseFilter.CoCreateInstance (CLSID_EnhancedVideoRenderer);

            if (SUCCEEDED (hr))   hr = graphBuilder->AddFilter (baseFilter, L"EVR");
            if (SUCCEEDED (hr))   hr = baseFilter.QueryInterface (getService);
            if (SUCCEEDED (hr))   hr = getService->GetService (MR_VIDEO_RENDER_SERVICE, IID_IMFVideoDisplayControl,
                                                               (LPVOID*) videoDisplayControl.resetAndGetPointerAddress());
            if (SUCCEEDED (hr))   hr = videoDisplayControl->SetVideoWindow (hwnd);
            if (SUCCEEDED (hr))   hr = videoDisplayControl->SetAspectRatioMode (MFVideoARMode_PreservePicture);

            return hr;
        }

        void setVideoWindow (HWND hwnd)
        {
            videoDisplayControl->SetVideoWindow (hwnd);
        }

        void setVideoPosition (HWND hwnd, long /*videoWidth*/, long /*videoHeight*/)
        {
            const MFVideoNormalizedRect src = { 0.0f, 0.0f, 1.0f, 1.0f };

            RECT dest;
            GetClientRect (hwnd, &dest);

            videoDisplayControl->SetVideoPosition (&src, &dest);
        }

        void repaintVideo (HWND /*hwnd*/, HDC /*hdc*/)
        {
            videoDisplayControl->RepaintVideo();
        }

        void displayModeChanged() {}

        HRESULT getVideoSize (long& videoWidth, long& videoHeight)
        {
            SIZE sz;
            HRESULT hr = videoDisplayControl->GetNativeVideoSize (&sz, nullptr);
            videoWidth  = sz.cx;
            videoHeight = sz.cy;
            return hr;
        }

    private:
        ComSmartPtr <IMFVideoDisplayControl> videoDisplayControl;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EVR)
    };
#endif
}


//======================================================================
class DirectShowComponent::DirectShowContext    : public AsyncUpdater
{
public:
    DirectShowContext (DirectShowComponent& c, VideoRendererType renderType)
        : component (c),
          hwnd (0),
          hdc (0),
          state (uninitializedState),
          hasVideo (false),
          videoWidth (0),
          videoHeight (0),
          type (renderType),
          needToUpdateViewport (true),
          needToRecreateNativeWindow (false)
    {
        CoInitialize (0);

        if (type == dshowDefault)
        {
            type = dshowVMR7;

           #if JUCE_MEDIAFOUNDATION
            if (SystemStats::getOperatingSystemType() >= SystemStats::WinVista)
                type = dshowEVR;
           #endif
        }
    }

    ~DirectShowContext()
    {
        release();
        CoUninitialize();
    }

    //======================================================================
    void updateWindowPosition (const Rectangle<int>& newBounds)
    {
        nativeWindow->setWindowPosition (newBounds);
    }

    void showWindow (bool shouldBeVisible)
    {
        nativeWindow->showWindow (shouldBeVisible);
    }

    //======================================================================
    void repaint()
    {
        if (hasVideo)
            videoRenderer->repaintVideo (nativeWindow->getHandle(), nativeWindow->getContext());
    }

    void updateVideoPosition()
    {
        if (hasVideo)
            videoRenderer->setVideoPosition (nativeWindow->getHandle(), videoWidth, videoHeight);
    }

    void displayResolutionChanged()
    {
        if (hasVideo)
            videoRenderer->displayModeChanged();
    }

    //======================================================================
    void peerChanged()
    {
        deleteNativeWindow();

        mediaEvent->SetNotifyWindow (0, 0, 0);
        if (videoRenderer != nullptr)
            videoRenderer->setVideoWindow (nullptr);

        juce::String err;
        createNativeWindow(err);

        mediaEvent->SetNotifyWindow ((OAHWND) hwnd, graphEventID, 0);
        if (videoRenderer != nullptr)
            videoRenderer->setVideoWindow (hwnd);
    }

    void handleAsyncUpdate() override
    {
        if (hwnd != 0)
        {
            if (needToRecreateNativeWindow)
            {
                peerChanged();
                needToRecreateNativeWindow = false;
            }

            if (needToUpdateViewport)
            {
                updateVideoPosition();
                needToUpdateViewport = false;
            }

            repaint();
        }
        else
        {
            triggerAsyncUpdate();
        }
    }

    void recreateNativeWindowAsync()
    {
        needToRecreateNativeWindow = true;
        triggerAsyncUpdate();
    }

    void updateContextPosition()
    {
        needToUpdateViewport = true;
        triggerAsyncUpdate();
    }
    juce::String ErrorAsString(DWORD errorMessageID=::GetLastError())
    {

        if(errorMessageID == 0)
            return std::string(); //No error message has been recorded

        LPSTR messageBuffer = nullptr;
        size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                     NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

        juce::String message(messageBuffer, size);

        //Free the buffer.
        LocalFree(messageBuffer);

        return message;
    }
    //======================================================================
    bool loadFile (const String& fileOrURLPath, String& err)
    {
        jassert (state == uninitializedState);

        err="createNativeWindow";
        if (! createNativeWindow(err))
        {
            return false;
        }

        err="CoCreateInstance CLSID_FilterGraph";
        HRESULT hr = graphBuilder.CoCreateInstance (CLSID_FilterGraph);

        // basic playback interfaces
        if (SUCCEEDED (hr))
        {
            err="QueryInterface mediaControl";
            hr = graphBuilder.QueryInterface (mediaControl);
        }
        else{
            switch(hr)
            {
//            case E_POINTER:
//                err="E_POINTER";
//                break;
            case E_NOINTERFACE:
                err="E_NOINTERFACE";
                break;
            case CLASS_E_NOAGGREGATION:
                err="CLASS_E_NOAGGREGATION";
                break;
            case REGDB_E_CLASSNOTREG:
                err="REGDB_E_CLASSNOTREG";
                break;
            }
            err=ErrorAsString(hr);
            release(); return false;
        }

        if (SUCCEEDED (hr))
        {
            err="QueryInterface mediaPosition";
            hr = graphBuilder.QueryInterface (mediaPosition);
        }
        else{ release(); return false;}
        if (SUCCEEDED (hr))
        {
            err="QueryInterface mediaEvent";
            hr = graphBuilder.QueryInterface (mediaEvent);
        }
        else{ release(); return false;}
        if (SUCCEEDED (hr))
        {
            err="QueryInterface basicAudio";
            hr = graphBuilder.QueryInterface (basicAudio);
        }
        else{ release(); return false;}

        // video renderer interface
        if (SUCCEEDED (hr))
        {
            err="create";
           #if JUCE_MEDIAFOUNDATION
            if (type == dshowEVR)
                videoRenderer = new DirectShowHelpers::EVR();
            else
           #endif
                videoRenderer = new DirectShowHelpers::VMR7();

            hr = videoRenderer->create (graphBuilder, baseFilter, hwnd, err);
            if (!SUCCEEDED (hr)){err=ErrorAsString(hr);delete videoRenderer;videoRenderer=0;release(); return false;}
        }
        else{ release(); return false;}

        // build filter graph
        if (SUCCEEDED (hr))
        {
            if(true)
            {
                if(!createFilterGraph(fileOrURLPath.toWideCharPointer(), err))
                {
                    release();
                    return false;
                }
            }
            else
            {
                err="RenderFile";
                hr = graphBuilder->RenderFile (fileOrURLPath.toWideCharPointer(), nullptr);

                if (FAILED (hr))
                {
                    // Annoyingly, if we don't run the msg loop between failing and deleting the window, the
                    // whole OS message-dispatch system gets itself into a state, and refuses to deliver any
                    // more messages for the whole app. (That's what happens in Win7, anyway)
                    MessageManager::getInstance()->runDispatchLoopUntil (200);
                }
            }
        }
        else{ release(); return false;}

        // remove video renderer if not connected (no video)
        if (SUCCEEDED (hr))
        {
            if (isRendererConnected())
            {
                err="getVideoSize";
                hasVideo = true;
                hr = videoRenderer->getVideoSize (videoWidth, videoHeight);
            }
            else
            {
                hasVideo = false;
                graphBuilder->RemoveFilter (baseFilter);
                videoRenderer = nullptr;
                baseFilter = nullptr;
            }
        }
        else{ release(); return false;}

        // set window to receive events
        if (SUCCEEDED (hr))
        {
            err="SetNotifyWindow";
            hr = mediaEvent->SetNotifyWindow ((OAHWND) hwnd, graphEventID, 0);
        }
        else{ release(); return false;}

        if (SUCCEEDED (hr))
        {
            state = stoppedState;
            pause();
            return true;
        }

        release();
        return false;
    }
    bool createFilterGraph(juce::String const& fileOrURLPath, juce::String & err)
    {/*
        const IID LAVSplitterSourceIID = uuidFromString("B98D13E7-55DB-4385-A33D-09FD1BA26338");

        IUnknown* pUnkSourceFilter = NULL;
        TCHAR* splitterPath=L"lav-filters64\\LAVSplitter.ax";
        HRESULT hr = CreateObjectFromPath(splitterPath, LAVSplitterSourceIID, &pUnkSourceFilter);
        if (FAILED(hr))
        {
            err = ErrorAsString(hr);
            err += "Unable create splitter source instance from ";
            err += splitterPath;
            return 0;
        }

        // Create an AVI splitter filter
        IFileSourceFilter* pLAVSplitterSource = NULL;
        hr = pUnk->QueryInterface (__uuidof(IFileSourceFilter), (void**)&pLAVSplitterSource);//{56A868A6-0AD4-11CE-B03A-0020AF0BA770}
        if (FAILED(hr))
        {
            err = ErrorAsString(hr);
            err += "Unable create query interface from instance of ";
            err += splitterPath;
            return 0;
        }


        SAFE_RELEASE(pLAVSplitterSource);
        SAFE_RELEASE(pUnkSourceFilter);
        */
        // Create a source filter
        IBaseFilter*	pSource= NULL;
        if(FAILED(graphBuilder->AddSourceFilter(fileOrURLPath.toWideCharPointer(),0,&pSource)))
        {
            err = "Unable to create source filter";
            return 0;
        }

        const GUID MEDIATYPE_Stream = {0xE436EB83, 0x524F, 0x11CE, {0x9F, 0x53, 0x00, 0x20, 0xAF, 0x0B, 0xA7, 0x70}};

        IPin* pSourceOut= GetPin(pSource, PINDIR_OUTPUT, &MEDIATYPE_Stream);
        if (!pSourceOut)
        {

            err = "Unable to obtain source pin";
            return 0;
        }


        const IID LAVSplitterIID = {0x171252A0,0x8820,0x4AFE, {0x9D,0xF8,0x5C,0x92,0xB2,0xD6,0x6B,0x04}};

        IUnknown* pUnk = NULL;
        TCHAR* splitterPath=L"lav-filters64\\LAVSplitter.ax";
        HRESULT hr = CreateObjectFromPath(splitterPath, LAVSplitterIID, &pUnk);
        if (FAILED(hr))
        {
            err = ErrorAsString(hr);
            err += "Unable create splitter instance from ";
            err += splitterPath;
            return 0;
        }


        // Create an AVI splitter filter
        IBaseFilter* pAVISplitter = NULL;
        hr = pUnk->QueryInterface (__uuidof(IBaseFilter), (void**)&pAVISplitter);
        if (FAILED(hr))
        {
            err = ErrorAsString(hr);
            err += "Unable create query interface from instance of ";
            err += splitterPath;
            return 0;
        }
        //if(FAILED(CoCreateInstance(CLSID_AviSplitter, NULL, CLSCTX_INPROC_SERVER,
        //                           IID_IBaseFilter, (void**)&pAVISplitter)) || !pAVISplitter)
        //{
        //    err = "Unable to create AVI splitter";
        //    return 0;
        //}

        IPin* pAVIsIn= GetPin(pAVISplitter, PINDIR_INPUT, &MEDIATYPE_Stream);
        if (!pAVIsIn)
        {

            err = "Unable to obtain input splitter pin";
            return 0;
        }

        // Connect the source and the splitter
        if(FAILED(graphBuilder->AddFilter( pAVISplitter, L"Splitter")) ||
                FAILED(graphBuilder->Connect(pSourceOut, pAVIsIn)) )
        {
            err = "Unable to connect AVI splitter filter";
            return 0;
        }

        ///////////////////////////////////////////////////////////////////////////////
        // VIDEO decoder filter
        ///////////////////////////////////////////////////////////////////////////////
        const GUID MEDIATYPE_Video  = {0x73646976, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}};
        const IID LAVVideoDecoderIID = {0xEE30215D,0x164F,0x4A92,{0xA4,0xEB,0x9D,0x4C,0x13,0x39,0x0F,0x9F}};

        IUnknown* pUnkVideo = NULL;
        TCHAR* videoDecoderPath=L"lav-filters64\\LAVVideo.ax";
        hr = CreateObjectFromPath(videoDecoderPath, LAVVideoDecoderIID, &pUnkVideo);
        if (FAILED(hr))
        {
            err = ErrorAsString(hr);
            err += "Unable create video decoder from ";
            err += videoDecoderPath;
            return 0;
        }
        IBaseFilter* pAVIDec = NULL;
        hr = pUnkVideo->QueryInterface (__uuidof(IBaseFilter), (void**)&pAVIDec);
        if (FAILED(hr))
        {
            err = ErrorAsString(hr);
            err += "Unable create query interface from instance of ";
            err += splitterPath;
            return 0;
        }

        IPin* pAVIsOut= GetPin(pAVISplitter, PINDIR_OUTPUT, &MEDIATYPE_Video);
        if (!pAVIsOut)
        {

            err = "Unable to obtain output splitter pin";
            return 0;
        }

        IPin* pAVIDecIn= GetPin(pAVIDec, PINDIR_INPUT, &MEDIATYPE_Video);
        if (!pAVIDecIn)
        {

            err = "Unable to obtain decoder input pin";
            return 0;
        }

        // Connect the splitter and the decoder
        if(FAILED(graphBuilder->AddFilter( pAVIDec, L"VideoDecoder")) ||
                FAILED(graphBuilder->Connect(pAVIsOut, pAVIDecIn)) )
        {
            err = "Unable to connect AVI decoder filter";
            return 0;
        }

        // Render the stream from the decoder
        IPin* pAVIDecOut= GetPin(pAVIDec, PINDIR_OUTPUT);
        if (!pAVIDecOut)
        {

            err = "Unable to obtain decoder output pin";
            return 0;
        }

        if(FAILED(graphBuilder->Render( pAVIDecOut )))
        {
            err = "Unable to connect to renderer";
            return 0;
        }
/*
        ///////////////////////////////////////////////////////////////////////////////
        // AUDIO decoder filter
        ///////////////////////////////////////////////////////////////////////////////
        const GUID MEDIATYPE_Audio  = {0x73647561, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}};
        const IID LAVAudioDecoderIID = uuidFromString("E8E73B6B-4CB3-44A4-BE99-4F7BCB96E491");

        IUnknown* pUnkAudio = NULL;
        TCHAR* audioDecoderPath=L"lav-filters64\\LAVAudio.ax";
        hr = CreateObjectFromPath(audioDecoderPath, LAVAudioDecoderIID, &pUnkVideo);
        if (FAILED(hr))
        {
            err = ErrorAsString(hr);
            err += "Unable create Audio decoder from ";
            err += audioDecoderPath;
            return 0;
        }
        IBaseFilter* pAudioDec = NULL;
        hr = pUnkAudio->QueryInterface (__uuidof(IBaseFilter), (void**)&pAudioDec);
        if (FAILED(hr))
        {
            err = ErrorAsString(hr);
            err += "Unable create query interface from instance of ";
            err += splitterPath;
            return 0;
        }

        IPin* pSplitterAudioOut= GetPin(pAVISplitter, PINDIR_OUTPUT, &MEDIATYPE_Audio);
        if (!pSplitterAudioOut)
        {

            err = "Unable to obtain Audio output splitter pin";
            return 0;
        }

        IPin* pAudioDecIn= GetPin(pAudioDec, PINDIR_INPUT, &MEDIATYPE_Audio);
        if (!pAudioDecIn)
        {

            err = "Unable to obtain Audio decoder input pin";
            return 0;
        }

        // Connect the splitter and the decoder
        if(FAILED(graphBuilder->AddFilter( pAudioDec, L"AudioDecoder")) ||
                FAILED(graphBuilder->Connect(pSplitterAudioOut, pAVIDecIn)) )
        {
            err = "Unable to connect Audio decoder filter";
            return 0;
        }

        // Render the stream from the decoder
        IPin* pAudioDecOut= GetPin(pAudioDec, PINDIR_OUTPUT);
        if (!pAudioDecOut)
        {

            err = "Unable to obtain audio decoder output pin";
            return 0;
        }

        if(FAILED(graphBuilder->Render( pAudioDecOut )))
        {
            err = "Unable to connect to renderer";
            return 0;
        }
*/
#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }
        SAFE_RELEASE(pAVIDecIn);
        SAFE_RELEASE(pAVIDecOut);
        SAFE_RELEASE(pAVIDec);
        SAFE_RELEASE(pAVIsOut);

        SAFE_RELEASE(pAVIsIn);
        SAFE_RELEASE(pAVISplitter);
/*

        SAFE_RELEASE(pUnkAudio);
        SAFE_RELEASE(pAudioDec);
        SAFE_RELEASE(pAudioDecIn);
        SAFE_RELEASE(pAudioDecOut);
        SAFE_RELEASE(pSplitterAudioOut);*/

        SAFE_RELEASE(pSourceOut);
        SAFE_RELEASE(pSource);
        SAFE_RELEASE(pUnk);
        SAFE_RELEASE(pUnkVideo);

        return 1;
    }
    void release()
    {
        if (mediaControl != nullptr)
            mediaControl->Stop();

        if (mediaEvent != nullptr)
            mediaEvent->SetNotifyWindow (0, 0, 0);

        if (videoRenderer != nullptr)
            videoRenderer->setVideoWindow (0);

        hasVideo = false;
        videoRenderer = nullptr;

        baseFilter = nullptr;
        basicAudio = nullptr;
        mediaEvent = nullptr;
        mediaPosition = nullptr;
        mediaControl = nullptr;
        graphBuilder = nullptr;

        state = uninitializedState;

        videoWidth = 0;
        videoHeight = 0;

        if (nativeWindow != nullptr)
            deleteNativeWindow();
    }

    void graphEventProc()
    {
        LONG ec;
        LONG_PTR p1, p2;

        jassert (mediaEvent != nullptr);

        while (SUCCEEDED (mediaEvent->GetEvent (&ec, &p1, &p2, 0)))
        {
            switch (ec)
            {
            case EC_REPAINT:
                component.repaint();
                break;

            case EC_COMPLETE:
                if (component.isLooping())
                    component.goToStart();
                else
                    component.stop();
                break;

            case EC_USERABORT:
            case EC_ERRORABORT:
            case EC_ERRORABORTEX:
                component.closeMovie();
                break;

            default:
                break;
            }

            mediaEvent->FreeEventParams (ec, p1, p2);
        }
    }

    //======================================================================
    void run()
    {
        mediaControl->Run();
        state = runningState;
    }

    void stop()
    {
        mediaControl->Stop();
        state = stoppedState;
    }

    void pause()
    {
        mediaControl->Pause();
        state = pausedState;
    }

    //======================================================================
    bool isInitialised() const noexcept  { return state != uninitializedState; }
    bool isRunning() const noexcept      { return state == runningState; }
    bool isPaused() const noexcept       { return state == pausedState; }
    bool isStopped() const noexcept      { return state == stoppedState; }
    bool containsVideo() const noexcept  { return hasVideo; }
    int getVideoWidth() const noexcept   { return (int) videoWidth; }
    int getVideoHeight() const noexcept  { return (int) videoHeight; }

    //======================================================================
    double getDuration() const
    {
        REFTIME duration;
        mediaPosition->get_Duration (&duration);
        return duration;
    }

    double getPosition() const
    {
        REFTIME seconds;
        mediaPosition->get_CurrentPosition (&seconds);
        return seconds;
    }

    //======================================================================
    void setSpeed (const float newSpeed)        { mediaPosition->put_Rate (newSpeed); }
    void setPosition (const double seconds)     { mediaPosition->put_CurrentPosition (seconds); }
    void setVolume (const float newVolume)      { basicAudio->put_Volume (convertToDShowVolume (newVolume)); }

    // in DirectShow, full volume is 0, silence is -10000
    static long convertToDShowVolume (const float vol) noexcept
    {
        if (vol >= 1.0f) return 0;
        if (vol <= 0.0f) return -10000;

        return roundToInt ((vol * 10000.0f) - 10000.0f);
    }

    float getVolume() const
    {
        long volume;
        basicAudio->get_Volume (&volume);
        return (volume + 10000) / 10000.0f;
    }

private:
    //======================================================================
    enum { graphEventID = WM_APP + 0x43f0 };

    DirectShowComponent& component;
    HWND hwnd;
    HDC hdc;

    enum State { uninitializedState, runningState, pausedState, stoppedState };
    State state;

    bool hasVideo;
    long videoWidth, videoHeight;

    VideoRendererType type;

    ComSmartPtr <IGraphBuilder> graphBuilder;
    ComSmartPtr <IMediaControl> mediaControl;
    ComSmartPtr <IMediaPosition> mediaPosition;
    ComSmartPtr <IMediaEventEx> mediaEvent;
    ComSmartPtr <IBasicAudio> basicAudio;
    ComSmartPtr <IBaseFilter> baseFilter;

    ScopedPointer <DirectShowHelpers::VideoRenderer> videoRenderer;

    bool needToUpdateViewport, needToRecreateNativeWindow;

    //======================================================================
    class NativeWindowClass   : private DeletedAtShutdown
    {
    public:
        bool isRegistered() const noexcept              { return atom != 0; }
        LPCTSTR getWindowClassName() const noexcept     { return (LPCTSTR) MAKELONG (atom, 0); }

        juce_DeclareSingleton_SingleThreaded_Minimal (NativeWindowClass);

    private:
        NativeWindowClass()
            : atom (0)
        {
            String windowClassName ("JUCE_DIRECTSHOW_");
            windowClassName << (int) (Time::currentTimeMillis() & 0x7fffffff);

            HINSTANCE moduleHandle = (HINSTANCE) Process::getCurrentModuleInstanceHandle();

            TCHAR moduleFile [1024] = { 0 };
            GetModuleFileName (moduleHandle, moduleFile, 1024);

            WNDCLASSEX wcex = { 0 };
            wcex.cbSize         = sizeof (wcex);
            wcex.style          = CS_OWNDC;
            wcex.lpfnWndProc    = (WNDPROC) wndProc;
            wcex.lpszClassName  = windowClassName.toWideCharPointer();
            wcex.hInstance      = moduleHandle;

            atom = RegisterClassEx (&wcex);
            jassert (atom != 0);
        }

        ~NativeWindowClass()
        {
            if (atom != 0)
                UnregisterClass (getWindowClassName(), (HINSTANCE) Process::getCurrentModuleInstanceHandle());

            clearSingletonInstance();
        }

        static LRESULT CALLBACK wndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
        {
            if (DirectShowContext* const c = (DirectShowContext*) GetWindowLongPtr (hwnd, GWLP_USERDATA))
            {
                switch (msg)
                {
                    case WM_NCHITTEST:          return HTTRANSPARENT;
                    case WM_ERASEBKGND:         return 1;
                    case WM_DISPLAYCHANGE:      c->displayResolutionChanged(); break;
                    case graphEventID:          c->graphEventProc(); return 0;
                    default:                    break;
                }
            }

            return DefWindowProc (hwnd, msg, wParam, lParam);
        }

        ATOM atom;

        JUCE_DECLARE_NON_COPYABLE (NativeWindowClass)
    };

    //======================================================================
    class NativeWindow
    {
    public:
        NativeWindow (HWND parentToAddTo, void* const userData)
            : hwnd (0), hdc (0)
        {
            NativeWindowClass* const wc = NativeWindowClass::getInstance();

            if (wc->isRegistered())
            {
                DWORD exstyle = 0;
                DWORD type = WS_CHILD;

                hwnd = CreateWindowEx (exstyle, wc->getWindowClassName(),
                                       L"", type, 0, 0, 0, 0, parentToAddTo, 0,
                                       (HINSTANCE) Process::getCurrentModuleInstanceHandle(), 0);

                if (hwnd != 0)
                {
                    hdc = GetDC (hwnd);
                    SetWindowLongPtr (hwnd, GWLP_USERDATA, (LONG_PTR) userData);
                }
            }

            jassert (hwnd != 0);
        }

        ~NativeWindow()
        {
            if (hwnd != 0)
            {
                SetWindowLongPtr (hwnd, GWLP_USERDATA, (LONG_PTR) 0);
                DestroyWindow (hwnd);
            }
        }

        HWND getHandle() const noexcept   { return hwnd; }
        HDC getContext() const noexcept   { return hdc; }

        void setWindowPosition (const Rectangle<int>& newBounds)
        {
            SetWindowPos (hwnd, 0, newBounds.getX(), newBounds.getY(),
                          newBounds.getWidth(), newBounds.getHeight(),
                          SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);
        }

        void showWindow (const bool shouldBeVisible)
        {
            ShowWindow (hwnd, shouldBeVisible ? SW_SHOWNA : SW_HIDE);
        }

    private:
        HWND hwnd;
        HDC hdc;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeWindow)
    };

    ScopedPointer<NativeWindow> nativeWindow;

    //======================================================================
    bool createNativeWindow(String& err)
    {
        jassert (nativeWindow == nullptr);

        if (ComponentPeer* const topLevelPeer = component.getTopLevelComponent()->getPeer())
        {
            nativeWindow = new NativeWindow ((HWND) topLevelPeer->getNativeHandle(), this);

            hwnd = nativeWindow->getHandle();

            if (hwnd != 0)
            {
                hdc = GetDC (hwnd);
                component.updateContextPosition();
                component.showContext (component.isShowing());
                return true;
            }
            else
            {
                err = "getHandle == null" ;
                nativeWindow = nullptr;
            }
        }
        else
        {
            err = "no toplevel" ;
            jassertfalse;
        }

        return false;
    }

    void deleteNativeWindow()
    {
        jassert (nativeWindow != nullptr);
        ReleaseDC (hwnd, hdc);
        hwnd = 0;
        hdc = 0;
        nativeWindow = nullptr;
    }

    bool isRendererConnected()
    {
        ComSmartPtr <IEnumPins> enumPins;

        HRESULT hr = baseFilter->EnumPins (enumPins.resetAndGetPointerAddress());

        if (SUCCEEDED (hr))
            hr = enumPins->Reset();

        ComSmartPtr<IPin> pin;

        while (SUCCEEDED (hr)
                && enumPins->Next (1, pin.resetAndGetPointerAddress(), nullptr) == S_OK)
        {
            ComSmartPtr<IPin> otherPin;

            hr = pin->ConnectedTo (otherPin.resetAndGetPointerAddress());

            if (SUCCEEDED (hr))
            {
                PIN_DIRECTION direction;
                hr = pin->QueryDirection (&direction);

                if (SUCCEEDED (hr) && direction == PINDIR_INPUT)
                    return true;
            }
            else if (hr == VFW_E_NOT_CONNECTED)
            {
                hr = S_OK;
            }
        }

        return false;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DirectShowContext)
};

juce_ImplementSingleton_SingleThreaded (DirectShowComponent::DirectShowContext::NativeWindowClass);


//======================================================================
class DirectShowComponent::DirectShowComponentWatcher   : public ComponentMovementWatcher
{
public:
    DirectShowComponentWatcher (DirectShowComponent* const c)
        : ComponentMovementWatcher (c),
          owner (c)
    {
    }

    void componentMovedOrResized (bool /*wasMoved*/, bool /*wasResized*/) override
    {
        if (owner->videoLoaded)
            owner->updateContextPosition();
    }

    void componentPeerChanged() override
    {
        if (owner->videoLoaded)
            owner->recreateNativeWindowAsync();
    }

    void componentVisibilityChanged() override
    {
        if (owner->videoLoaded)
            owner->showContext (owner->isShowing());
    }

private:
    DirectShowComponent* const owner;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DirectShowComponentWatcher)
};


//======================================================================
DirectShowComponent::DirectShowComponent (VideoRendererType type)
    : videoLoaded (false),
      looping (false)
{
    setOpaque (true);
    context = new DirectShowContext (*this, type);
    componentWatcher = new DirectShowComponentWatcher (this);
}

DirectShowComponent::~DirectShowComponent()
{
    componentWatcher = nullptr;
}

bool DirectShowComponent::isDirectShowAvailable()
{
    static bool isDSAvailable = DirectShowHelpers::checkDShowAvailability();
    return isDSAvailable;
}

void DirectShowComponent::recreateNativeWindowAsync()
{
    context->recreateNativeWindowAsync();
    repaint();
}

void DirectShowComponent::updateContextPosition()
{
    context->updateContextPosition();

    if (getWidth() > 0 && getHeight() > 0)
        if (ComponentPeer* peer = getTopLevelComponent()->getPeer())
            context->updateWindowPosition (peer->getAreaCoveredBy (*this));
}

void DirectShowComponent::showContext (const bool shouldBeVisible)
{
    context->showWindow (shouldBeVisible);
}

void DirectShowComponent::paint (Graphics& g)
{
    if (videoLoaded)
        context->handleUpdateNowIfNeeded();
    else
        g.fillAll (Colours::grey);
}

//======================================================================
bool DirectShowComponent::loadMovie (const String& fileOrURLPath, String &err)
{
    closeMovie();

    videoLoaded = context->loadFile (fileOrURLPath, err);

    if (videoLoaded)
    {
        videoPath = fileOrURLPath;
        context->updateVideoPosition();
    }

    return videoLoaded;
}

bool DirectShowComponent::loadMovie (const File& videoFile, String &err)
{
    return loadMovie (videoFile.getFullPathName(), err);
}

bool DirectShowComponent::loadMovie (const URL& videoURL, String &err)
{
    return loadMovie (videoURL.toString (false), err);
}

void DirectShowComponent::closeMovie()
{
    if (videoLoaded)
        context->release();

    videoLoaded = false;
    videoPath = String::empty;
}

//======================================================================
File DirectShowComponent::getCurrentMoviePath() const           { return videoPath; }
bool DirectShowComponent::isMovieOpen() const                   { return videoLoaded; }
double DirectShowComponent::getMovieDuration() const            { return videoLoaded ? context->getDuration() : 0.0; }
void DirectShowComponent::setLooping (const bool shouldLoop)    { looping = shouldLoop; }
bool DirectShowComponent::isLooping() const                     { return looping; }

void DirectShowComponent::getMovieNormalSize (int &width, int &height) const
{
    width = context->getVideoWidth();
    height = context->getVideoHeight();
}

//======================================================================
void DirectShowComponent::setBoundsWithCorrectAspectRatio (const Rectangle<int>& spaceToFitWithin,
                                                           RectanglePlacement placement)
{
    int normalWidth, normalHeight;
    getMovieNormalSize (normalWidth, normalHeight);

    const Rectangle<int> normalSize (0, 0, normalWidth, normalHeight);

    if (! (spaceToFitWithin.isEmpty() || normalSize.isEmpty()))
        setBounds (placement.appliedTo (normalSize, spaceToFitWithin));
    else
        setBounds (spaceToFitWithin);
}

//======================================================================
void DirectShowComponent::play()
{
    if (videoLoaded)
        context->run();
}

void DirectShowComponent::stop()
{
    if (videoLoaded)
        context->pause();
}

bool DirectShowComponent::isPlaying() const
{
    return context->isRunning();
}

void DirectShowComponent::goToStart()
{
    setPosition (0.0);
}

void DirectShowComponent::setPosition (const double seconds)
{
    if (videoLoaded)
        context->setPosition (seconds);
}

double DirectShowComponent::getPosition() const
{
    return videoLoaded ? context->getPosition() : 0.0;
}

void DirectShowComponent::setSpeed (const float newSpeed)
{
    if (videoLoaded)
        context->setSpeed (newSpeed);
}

void DirectShowComponent::setMovieVolume (const float newVolume)
{
    if (videoLoaded)
        context->setVolume (newVolume);
}

float DirectShowComponent::getMovieVolume() const
{
    return videoLoaded ? context->getVolume() : 0.0f;
}
