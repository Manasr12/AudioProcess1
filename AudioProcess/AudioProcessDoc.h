
// AudioProcessDoc.h : interface of the CAudioProcessDoc class
//


#pragma once

#include "Progress.h"
#include "audio/wave.h"
#include "audio/DirSoundStream.h"
#include "audio/DirSoundSource.h"
#include "audio/WaveformBuffer.h"

class CAudioProcessDoc : public CDocument, private CProgress
{
protected: // create from serialization only
	CAudioProcessDoc();
	DECLARE_DYNCREATE(CAudioProcessDoc)

// Attributes
public:
    int SampleFrames() const {return m_numSampleFrames;}
    double SampleRate() const {return m_sampleRate;}
    int NumChannels() const {return m_numChannels;}
    double Amplitude() const {return m_amplitude;}

    CWaveformBuffer *GetWaveformBuffer() {return &m_waveformBuffer;}

	// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CAudioProcessDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS

private:
	void ProcessReadFrame(short *p_frame);
	bool m_fileoutput;
	bool m_audiooutput;
	void ProcessWriteFrame(short *p_frame);
	bool OpenProcessFile(CWaveOut &p_wave);
	void ProcessEnd();
	bool ProcessBegin();
    bool ProcessProgress(double p) {return Progress(p);}

    CDirSoundSource   m_wavein;
    CWaveOut          m_waveout;
    CWaveformBuffer m_waveformBuffer;

    double   m_amplitude;
    double   m_sampleRate;
    int      m_numChannels;
    int      m_numSampleFrames;

    CDirSoundStream  m_soundstream;
	struct FTerm {
		int m_delay;
		double m_weight;
	};
	std::list<FTerm> m_xterms;
	std::list<FTerm> m_yterms;
	double m_f = 0;
	double m_b = 0;
public:
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	afx_msg void OnProcessFileoutput();
	afx_msg void OnProcessAudiooutput();
	afx_msg void OnUpdateProcessFileoutput(CCmdUI *pCmdUI);
	afx_msg void OnUpdateProcessAudiooutput(CCmdUI *pCmdUI);
	afx_msg void OnProcessCopy();
	afx_msg void OnProcessParameters();
	afx_msg void OnRampRamp();
	afx_msg void OnRampRampin();
	afx_msg void OnTremeloTremelo();
	afx_msg void OnHalfspeedHalfspeed();
	afx_msg void OnDoublespeedDoublespeed();
	afx_msg void OnBackwardsBackwards();
	afx_msg void OnTransferloadTransferload();
	afx_msg void OnFilterFilter();
	afx_msg void OnResondialogResondialog();
	afx_msg void OnResonfilterResonfilter();
};
