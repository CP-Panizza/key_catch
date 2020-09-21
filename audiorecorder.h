/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef AUDIORECORDER_H
#define AUDIORECORDER_H




#include <QMediaRecorder>
#include <QSoundEffect>
#include <QUrl>
#include <QFile>
#include <QAudioInput>


class QAudioRecorder;
class QAudioProbe;
class QAudioBuffer;

class QAudioLevel;


struct WAVFILEHEADER
{
    // RIFF 头;
    char RiffName[4];
    unsigned long nRiffLength;

    // 数据类型标识符;
    char WavName[4];

    // 格式块中的块头;
    char FmtName[4];
    unsigned long nFmtLength;

    // 格式块中的块数据;
    unsigned short nAudioFormat;
    unsigned short nChannleNumber;
    unsigned long nSampleRate;
    unsigned long nBytesPerSecond;
    unsigned short nBytesPerSample;
    unsigned short nBitsPerSample;

    // 数据块中的块头;
    char    DATANAME[4];
    unsigned long   nDataLength;
};



class AudioRecorder {
public:
    AudioRecorder();
    ~AudioRecorder();
private slots:
    void on_recordButton_clicked();

    void on_stopButton_clicked();

    void setFilePath(const QString &value);

    QString& getFilePath();

    QString &getCacheFileName();
    void startRecord();
    void stopRecord();
    bool addWavHeader();

private:
    QAudioRecorder *audioRecorder;
    QAudioProbe *probe;
    QList<QAudioLevel*> audioLevels;


    QStringList m_fileNames;


    QString filePath = "";
    QFile cacheFile;
    int simpleRate = 16000;
    int simpleSize = 16;
    int channelCount = 1;
    QString codeC = "audio/pcm";

    QString outputFormat = "wav";

    QString cacheFileName = "tmp.wav";
    QAudioInput* m_audioInput = nullptr;
};

#endif // AUDIORECORDER_H
