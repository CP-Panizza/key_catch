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

#include <QAudioProbe>
#include <QAudioRecorder>
#include <QDir>
#include <QFileDialog>
#include <QMediaRecorder>
#include <QTextCodec>
#include <QMessageBox>

#include "audiorecorder.h"

AudioRecorder::AudioRecorder()
{
    audioRecorder = new QAudioRecorder(this);
    probe = new QAudioProbe;
    probe->setSource(audioRecorder);
}

AudioRecorder::~AudioRecorder()
{
    delete audioRecorder;
    delete probe;
}



void AudioRecorder::on_recordButton_clicked()
{
    this->startRecord();
}

void AudioRecorder::on_stopButton_clicked()
{

    QString fileName = QFileDialog::getSaveFileName(this,
            tr("output"),
            "",
            tr("Save Files (*.wav *.mp3)"));
    if (!fileName.isNull())
    {
        QStringList v = fileName.split(".");
        if(v.size() != 2){
            qDebug() << "File path not ok;\n" ;
            return;
        }
        this->setFilePath(v[0]);
    }
    else
    {
        if (this->m_audioInput != nullptr)
        {
            this->m_audioInput->stop();
            this->cacheFile.close();
            delete this->m_audioInput;
            this->m_audioInput = nullptr;
        }
        return;
    }
    this->stopRecord();
}




void AudioRecorder::setFilePath(const QString &value)
{
    filePath = value;
}

QString &AudioRecorder::getFilePath()
{
    return this->filePath;
}

QString &AudioRecorder::getCacheFileName()
{
    return this->cacheFileName;
}

void AudioRecorder::startRecord()
{
    // 判断本地设备是否支持该格式
    QAudioDeviceInfo audioDeviceInfo = QAudioDeviceInfo::defaultInputDevice();
    // 判断本地是否有录音设备;
    if (!audioDeviceInfo.isNull())
    {
        cacheFile.setFileName(this->cacheFileName);
        cacheFile.open(QIODevice::WriteOnly | QIODevice::Truncate);

        // 设置音频文件格式;
        QAudioFormat format;
        // 设置采样频率;
        format.setSampleRate(simpleRate);
        // 设置通道数;
        format.setChannelCount(channelCount);
        // 设置每次采样得到的样本数据位值;
        format.setSampleSize(simpleSize);
        // 设置编码方法;
        format.setCodec(codeC);
        // 设置采样字节存储顺序;
        format.setByteOrder(QAudioFormat::LittleEndian);
        // 设置采样类型;

        format.setSampleType(QAudioFormat::SignedInt);


        // 判断当前设备设置是否支持该音频格式;
        if (!audioDeviceInfo.isFormatSupported(format))
        {
            qDebug() << "Default format not supported, trying to use the nearest.";
            format = audioDeviceInfo.nearestFormat(format);
        }

        m_audioInput = new QAudioInput(format, this);
        m_audioInput->start(&cacheFile);
    }
    else
    {
        qDebug() << "Current No Record Device";
        // 没有录音设备;
        //                QMessageBox::information(NULL, tr("Record"), tr("Current No Record Device"));
    }
}

void AudioRecorder::stopRecord()
{
    if (m_audioInput != nullptr)
    {
        m_audioInput->stop();
        cacheFile.close();
        delete m_audioInput;
        m_audioInput = nullptr;
    }

    if(addWavHeader()){
        QMessageBox::information(NULL, tr("Record"), tr("save file success!"));
    } else {
         QMessageBox::information(NULL, tr("Record"), tr("save file err!"));
    }
}

bool AudioRecorder::addWavHeader()
{

    WAVFILEHEADER WavFileHeader;
    qstrcpy(WavFileHeader.RiffName, "RIFF");
    qstrcpy(WavFileHeader.WavName, "WAVE");
    qstrcpy(WavFileHeader.FmtName, "fmt ");

    WavFileHeader.nFmtLength = 16;  //  表示 FMT 的长度
    WavFileHeader.nAudioFormat = 1; //这个表示 PCM 编码;

    qstrcpy(WavFileHeader.DATANAME, "data");

    WavFileHeader.nBitsPerSample = simpleSize;
    WavFileHeader.nBytesPerSample = simpleSize/8;
    WavFileHeader.nSampleRate = simpleRate;
    WavFileHeader.nBytesPerSecond = simpleRate*simpleSize/8;
    WavFileHeader.nChannleNumber = channelCount;

    QFile wavFile(filePath+".wav");

    if (!cacheFile.open(QIODevice::ReadWrite))
    {
        return false;
    }
    if (!wavFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        return false;
    }

    int nSize = sizeof(WavFileHeader);
    qint64 nFileLen = cacheFile.bytesAvailable();

    WavFileHeader.nRiffLength = nFileLen - 8 + nSize;
    WavFileHeader.nDataLength = nFileLen;


    wavFile.write((char *)&WavFileHeader, nSize);
    wavFile.write(cacheFile.readAll());

    cacheFile.close();
    wavFile.close();

    return true;
}
