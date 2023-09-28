
// AudioProcessDoc.cpp : implementation of the CAudioProcessDoc class
//

#include "pch.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "AudioProcess.h"
#endif

#include "AudioProcessDoc.h"
#include "ProcessDlg.h"

#include <vector>
#include <fstream>

#include <propkey.h>
#include "filterdialog.h"
#include <iostream>

using namespace std;


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void CAudioProcessDoc::OnProcessCopy()
{
   // Call to open the processing output
   if(!ProcessBegin())
      return;

   short audio[2];

   for(int i=0;  i<SampleFrames();  i++)
   {                 
      ProcessReadFrame(audio);

      audio[0] = short(audio[0] * m_amplitude);
      audio[1] = short(audio[1] * m_amplitude);

      ProcessWriteFrame(audio);

      // The progress control
      if(!ProcessProgress(double(i) / SampleFrames()))
         break;
   }

   
   // Call to close the generator output
   ProcessEnd();
}



// CAudioProcessDoc

IMPLEMENT_DYNCREATE(CAudioProcessDoc, CDocument)

BEGIN_MESSAGE_MAP(CAudioProcessDoc, CDocument)
	ON_COMMAND(ID_PROCESS_FILEOUTPUT, &CAudioProcessDoc::OnProcessFileoutput)
	ON_COMMAND(ID_PROCESS_AUDIOOUTPUT, &CAudioProcessDoc::OnProcessAudiooutput)
	ON_UPDATE_COMMAND_UI(ID_PROCESS_FILEOUTPUT, &CAudioProcessDoc::OnUpdateProcessFileoutput)
	ON_UPDATE_COMMAND_UI(ID_PROCESS_AUDIOOUTPUT, &CAudioProcessDoc::OnUpdateProcessAudiooutput)
	ON_COMMAND(ID_PROCESS_COPY, &CAudioProcessDoc::OnProcessCopy)
	ON_COMMAND(ID_PROCESS_PARAMETERS, &CAudioProcessDoc::OnProcessParameters)
	ON_COMMAND(ID_RAMP_RAMP, &CAudioProcessDoc::OnRampRamp)
	ON_COMMAND(ID_RAMP_RAMPIN, &CAudioProcessDoc::OnRampRampin)
	ON_COMMAND(ID_TREMELO_TREMELO, &CAudioProcessDoc::OnTremeloTremelo)
	ON_COMMAND(ID_HALFSPEED_HALFSPEED, &CAudioProcessDoc::OnHalfspeedHalfspeed)
	ON_COMMAND(ID_DOUBLESPEED_DOUBLESPEED, &CAudioProcessDoc::OnDoublespeedDoublespeed)
	ON_COMMAND(ID_BACKWARDS_BACKWARDS, &CAudioProcessDoc::OnBackwardsBackwards)
	ON_COMMAND(ID_TRANSFERLOAD_TRANSFERLOAD, &CAudioProcessDoc::OnTransferloadTransferload)
	ON_COMMAND(ID_FILTER_FILTER, &CAudioProcessDoc::OnFilterFilter)
	ON_COMMAND(ID_RESONDIALOG_RESONDIALOG, &CAudioProcessDoc::OnResondialogResondialog)
	ON_COMMAND(ID_RESONFILTER_RESONFILTER, &CAudioProcessDoc::OnResonfilterResonfilter)
END_MESSAGE_MAP()




// CAudioProcessDoc construction/destruction

CAudioProcessDoc::CAudioProcessDoc()
{
   m_audiooutput = true;
   m_fileoutput = false;

   m_numChannels = 2;
   m_sampleRate = 44100.;
   m_numSampleFrames = 0;
   m_amplitude = 1.0;
}

CAudioProcessDoc::~CAudioProcessDoc()
{
}

BOOL CAudioProcessDoc::OnNewDocument()
{
	return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CAudioProcessDoc serialization

void CAudioProcessDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CAudioProcessDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CAudioProcessDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CAudioProcessDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CAudioProcessDoc diagnostics

#ifdef _DEBUG
void CAudioProcessDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CAudioProcessDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CAudioProcessDoc commands


BOOL CAudioProcessDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	
	if(!m_wavein.Open(lpszPathName))
		return FALSE;

	m_sampleRate = m_wavein.SampleRate();
	m_numChannels = m_wavein.NumChannels();
	m_numSampleFrames = m_wavein.NumSampleFrames();
   
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
// The following functions manage the audio processing process, 
// directing output to the waveform buffer, file, and/or audio 
// output.  
//
/////////////////////////////////////////////////////////////////////////////


//
// Name :        CAudioProcessDoc::ProcessBegin()
// Description : This function starts the audio processing process.
//               It opens the waveform storage, opens the file
//               if file output is requested, and opens the 
//               audio output if audio output is requested.
//               Be sure to call EndProcess() when done.
// Returns :     true if successful...
//

bool CAudioProcessDoc::ProcessBegin()
{
    m_wavein.Rewind();

	// 
	// Waveform storage
	//

	m_waveformBuffer.Start(NumChannels(), SampleRate());

    if(m_fileoutput)
    {
      if(!OpenProcessFile(m_waveout))
         return false;
    }

   ProgressBegin(this);

   if(m_audiooutput)
   {
      m_soundstream.SetChannels(NumChannels());
      m_soundstream.SetSampleRate(int(SampleRate()));

      m_soundstream.Open(((CAudioProcessApp *)AfxGetApp())->DirSound());
   }

   return true;
}


//
// Name :        CAudioProcessDoc::ProcessReadFrame()
// Description : Read a frame of input from the current audio file.
//

void CAudioProcessDoc::ProcessReadFrame(short *p_frame)
{
   m_wavein.ReadFrame(p_frame);
}


//
// Name :        CAudioProcessDoc::ProcessWriteFrame()
// Description : Write a frame of output to the current generation device.
//

void CAudioProcessDoc::ProcessWriteFrame(short *p_frame)
{
    m_waveformBuffer.Frame(p_frame);

   if(m_fileoutput)
      m_waveout.WriteFrame(p_frame);

   if(m_audiooutput)
      m_soundstream.WriteFrame(p_frame);
}


//
// Name :        CAudioProcessDoc::ProcessEnd()
// Description : End the generation process.
//

void CAudioProcessDoc::ProcessEnd()
{
    m_waveformBuffer.End();

   if(m_fileoutput)
      m_waveout.close();

   if(m_audiooutput)
      m_soundstream.Close();

   ProgressEnd(this);


}

//
// Name :        CAudioProcessDoc::OpenProcessFile()
// Description : This function opens the audio file for output.
// Returns :     true if successful...
//

bool CAudioProcessDoc::OpenProcessFile(CWaveOut &p_wave)
{
   p_wave.NumChannels(m_numChannels);
   p_wave.SampleRate(m_sampleRate);

	static WCHAR BASED_CODE szFilter[] = L"Wave Files (*.wav)|*.wav|All Files (*.*)|*.*||";

	CFileDialog dlg(FALSE, L".wav", NULL, 0, szFilter, NULL);
	if(dlg.DoModal() != IDOK)
      return false;

    p_wave.open(dlg.GetPathName());
   if(p_wave.fail())
      return false;

   return true;
}



void CAudioProcessDoc::OnProcessFileoutput()
{
	m_fileoutput = !m_fileoutput;
}


void CAudioProcessDoc::OnProcessAudiooutput()
{
   m_audiooutput = !m_audiooutput;
}


void CAudioProcessDoc::OnUpdateProcessFileoutput(CCmdUI *pCmdUI)
{
   pCmdUI->SetCheck(m_fileoutput);	
}


void CAudioProcessDoc::OnUpdateProcessAudiooutput(CCmdUI *pCmdUI)
{
   pCmdUI->SetCheck(m_audiooutput);	
}



void CAudioProcessDoc::OnProcessParameters()
{
   CProcessDlg dlg;
   
   dlg.m_amplitude = m_amplitude;

   if(dlg.DoModal() == IDOK)
   {
      m_amplitude = dlg.m_amplitude;
   }
}


void CAudioProcessDoc::OnRampRamp()
{
	// Call to open the processing output
	if (!ProcessBegin())
		return;

	short audio[2];
	double time = 0;

	for (int i = 0; i < SampleFrames(); i++, time += 1.0 / SampleRate())
	{
		ProcessReadFrame(audio);
		double ramp;
		if (time < 0.5)
		{
			ramp = time / 0.5;
		}
		else
		{
			ramp = 1;
		}
		audio[0] = short(audio[0] * m_amplitude * ramp);
		audio[1] = short(audio[1] * m_amplitude * ramp);

		ProcessWriteFrame(audio);

		// The progress control
		if (!ProcessProgress(double(i) / SampleFrames()))
			break;
	}


	// Call to close the generator output
	ProcessEnd();
}


void CAudioProcessDoc::OnRampRampin()
{

	if (!ProcessBegin())
		return;

	short audio[2];
	double time = 0;
	double inFade = 0.75;
	double outFade = 0.85;
	double Total = double(SampleFrames()) / SampleRate();
	int i = 0;
	while (i < SampleFrames())
	{
		ProcessReadFrame(audio);
		double ramp;
		if (time < inFade)
		{
			ramp = time / inFade;
		}
		else if (time > Total - outFade)
		{
			ramp = (Total - time) / outFade;
		}
		else
		{
			ramp = 1;
		}
		audio[0] = short(audio[0] * m_amplitude * ramp);
		audio[1] = short(audio[1] * m_amplitude * ramp);

		ProcessWriteFrame(audio);


		if (!ProcessProgress(double(i) / SampleFrames()))
			break;

		time += 1.0 / SampleRate();
		i++;
	}


	ProcessEnd();
}


void CAudioProcessDoc::OnTremeloTremelo()
{

	if (!ProcessBegin())
		return;

	short audio[2];
	double time = 0;
	double depth = 0.45;
	double frequency = 3.25;

	for (int i = 0; i < SampleFrames(); i++, time += 1.0 / SampleRate())
	{
		ProcessReadFrame(audio);
		audio[0] = short(audio[0] * 1 + depth * sin(frequency * 2 * M_PI * time));
		audio[1] = short(audio[1] * 1 + depth * sin(frequency * 2 * M_PI * time));

		ProcessWriteFrame(audio);

	
		if (!ProcessProgress(double(i) / SampleFrames()))
			break;
	}


	ProcessEnd();
}


void CAudioProcessDoc::OnHalfspeedHalfspeed()
{
	// Call to open the processing output
	if (!ProcessBegin())
		return;

	short audio[2];
	short audioNew[2];
	audioNew[0] = 0;
	audioNew[1] = 0;
	int i = 0;

	while (i < SampleFrames())
	{
		ProcessReadFrame(audio);

		ProcessWriteFrame(audioNew);

		short audioAverage[2];
		audioAverage[0] = short((audio[0] + audioNew[0]) / 2);
		audioAverage[1] = short((audio[1] + audioNew[1]) / 2);

		ProcessWriteFrame(audioAverage);

		audioNew[0] = audio[0];
		audioNew[1] = audio[1];


		if (!ProcessProgress(double(i) / SampleFrames()))
			break;

		i++;
	}

	// Call to close the generator output
	ProcessEnd();
}


void CAudioProcessDoc::OnDoublespeedDoublespeed()
{
	// Call to open the processing output
		if (!ProcessBegin())
			return;

	short audio1[2];
	short audio2[2];
	short output[2];
	int i = 0;

	while(i < SampleFrames())
	{
	
		ProcessReadFrame(audio1);
		ProcessReadFrame(audio2);


		output[0] = short((audio1[0] + audio2[0]) / 2);
		output[1] = short((audio1[1] + audio2[1]) / 2);
		ProcessWriteFrame(output);

		if (!ProcessProgress(double(i) / SampleFrames()))
			break;
		i += 2;
	}


	ProcessEnd();
}


void CAudioProcessDoc::OnBackwardsBackwards()
{
	if (!ProcessBegin())
		return;

	short* audio[2];
	audio[0] = new short[SampleFrames()];
	audio[1] = new short[SampleFrames()];
	short AllFrames[2];
	
	for (int i = 0; i < SampleFrames(); i++)
	{
		ProcessReadFrame(AllFrames);
		audio[0][i] = AllFrames[0];
		audio[1][i] = AllFrames[1];
	}

	for (int i = SampleFrames() - 1; i >= 0; i--)
	{
		AllFrames[0] = audio[0][i];
		AllFrames[1] = audio[1][i];
		ProcessWriteFrame(AllFrames);


		if (!ProcessProgress(double(SampleFrames() - i) / SampleFrames()))
			break;
	}


	ProcessEnd();

	delete audio[0];
	delete audio[1];
}


void CAudioProcessDoc::OnTransferloadTransferload()
{
	static WCHAR BASED_CODE szFilter[] = L"Filter Transfer Equation Files (*.tran)|*.tran|All Files (*.*)|*.*||";

	CFileDialog dlg(TRUE, L".tran", NULL, 0, szFilter, NULL);
	if (dlg.DoModal() != IDOK)
		return;

	ifstream filt(dlg.GetPathName());
	if (filt.fail())
	{
		AfxMessageBox(L"Failed to open filter transfer equation file");
		return;
	}
	m_xterms.clear();
	m_yterms.clear();

	int firstLine;

	if (filt >> firstLine) {
		int i = 0;
		while (i < firstLine && filt) {
			FTerm fterm;
			if (filt >> fterm.m_delay >> fterm.m_weight) {
				m_xterms.push_back(fterm);
				i++;
			}
			else {
				break; 
			}
		}
	}

	if (filt >> firstLine) {
		int i = 0;
		while (i < firstLine && filt) {
			FTerm fterm;
			if (filt >> fterm.m_delay >> fterm.m_weight) {
				m_yterms.push_back(fterm);
				i++;
			}
			else {
				break; 
			}
		}
	}

	filt.close();
	
}


void CAudioProcessDoc::OnFilterFilter()
{
	if (!ProcessBegin())
		return;

	
	const int qLength = static_cast<int>(SampleRate() * 20);
	std::vector<short> qX(qLength), qY(qLength);
	int writeIndex = 0;
	double currentTime = 0.0;

	int i = 0;
	while (i < SampleFrames())
	{
		short sound[2];
		ProcessReadFrame(sound);

		writeIndex = (writeIndex + 2) % qLength;
		qX[writeIndex] = sound[0] * m_amplitude;
		qX[writeIndex + 1] = sound[1] * m_amplitude;
	
		sound[0] = 0;
		sound[1] = 0;
		int tempVar[2];
		tempVar[0] = 0;
		tempVar[1] = 0;

		auto xTermIter = m_xterms.begin();
		auto yTermIter = m_yterms.begin();

		while (xTermIter != m_xterms.end() || yTermIter != m_yterms.end())
		{
			if (xTermIter != m_xterms.end())
			{
				int delayLength = int(xTermIter->m_delay) * 2;
				int eq = (writeIndex + qLength - delayLength) % qLength;
				tempVar[0] = tempVar[0] + int(qX[eq] * xTermIter->m_weight);
				tempVar[1] = tempVar[1] + int(qX[eq + 1] * xTermIter->m_weight);
				xTermIter++;
			}

			if (yTermIter != m_yterms.end())
			{
				int delayLength = int(yTermIter->m_delay) * 2;
				int eq = (writeIndex + qLength - delayLength) % qLength;
				tempVar[0] = tempVar[0] + int(qY[eq] * yTermIter->m_weight);
				tempVar[1] = tempVar[1] + int(qY[eq + 1] * yTermIter->m_weight);
				yTermIter++;
			}
		}

		sound[0] = short(float(tempVar[0]));
		sound[1] = short(float(tempVar[1]));
		qY[writeIndex] = sound[0];
		qY[writeIndex + 1] = sound[1];

		ProcessWriteFrame(sound);

		if (!ProcessProgress(double(i) / SampleFrames()))
			break;

		i++;
		currentTime += 1. / SampleRate();
	}

	ProcessEnd();
	
	
}


void CAudioProcessDoc::OnResondialogResondialog()
{
	filterdialog dlg;

	dlg.m_frequency = m_f;
	dlg.m_bandwidth = m_b;
	

	if (dlg.DoModal() == IDOK)
	{
		m_f = dlg.m_frequency;
		m_b = dlg.m_bandwidth;
	

		UpdateAllViews(NULL);
	}// TODO: Add your command handler code here
}


void CAudioProcessDoc::OnResonfilterResonfilter()
{
	double bandwidth = 0.01;
	double frequency = 0.045351;
	double dampingFactor = 1 - bandwidth / 2.0f;
	double pi = 3.1415926;
	double phaseAngle = acos((2 * dampingFactor * cos(2 * pi * frequency)) / (1 + dampingFactor * dampingFactor));
	double amplificationFactor = (1 - dampingFactor * dampingFactor) * sin(phaseAngle) * 10;

	if (!ProcessBegin())
		return;

	short audio[2];
	const int QUEUE_SIZE = int(SampleRate() * 20);
	vector<short> inputQueue, outputQueue;
	inputQueue.resize(QUEUE_SIZE);
	outputQueue.resize(QUEUE_SIZE);

	int writeLocation = 0;
	double time = 0;

	for (int frameIndex = 0; frameIndex < SampleFrames(); frameIndex++, time += 1. / SampleRate())
	{
		ProcessReadFrame(audio);

		writeLocation = (writeLocation + 2) % QUEUE_SIZE;
		inputQueue[writeLocation] = audio[0];
		inputQueue[writeLocation + 1] = audio[1];
		audio[0] = 0;
		audio[1] = 0;

		int temp[2];
		temp[0] = 0;
		temp[1] = 0;

		int delayIndex = 0;
		while (delayIndex < 3)
		{
			int delayLength = delayIndex * 2;
			int readLocation = (writeLocation + QUEUE_SIZE - delayLength) % QUEUE_SIZE;

			if (delayIndex == 0)
			{
				temp[0] += int(inputQueue[readLocation] * amplificationFactor);
				temp[1] += int(inputQueue[readLocation + 1] * amplificationFactor);
			}
			else if (delayIndex == 1)
			{
				temp[0] += int(outputQueue[readLocation] * 2 * dampingFactor * cos(phaseAngle));
				temp[1] += int(outputQueue[readLocation + 1] * 2 * dampingFactor * cos(phaseAngle));
			}
			else if (delayIndex == 2)
			{
				temp[0] += int(outputQueue[readLocation] * (-dampingFactor * dampingFactor));
				temp[1] += int(outputQueue[readLocation + 1] * (-dampingFactor * dampingFactor));
			}

			delayIndex++;
		}

		if (temp[0] > 32767)
			audio[0] = 32767;
		else if (temp[0] < -32768)
			audio[0] = -32768;
		else
			audio[0] = short(temp[0]);

		if (temp[1] > 32767)
			audio[1] = 32767;
		else if (temp[1] < -32768)
			audio[1] = -32768;
		else
			audio[1] = short(temp[1]);

		outputQueue[writeLocation] = audio[0];
		outputQueue[writeLocation + 1] = audio[1];

		ProcessWriteFrame(audio);
		if (!ProcessProgress(double(frameIndex) / SampleFrames()))
			break;
	}

	ProcessEnd();
}
