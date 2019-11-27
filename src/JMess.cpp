/*
  JMess: A simple utility so save your jack-audio mess.

  Copyright (C) 2007-2010 Juan-Pablo Caceres.

  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated documentation
  files (the "Software"), to deal in the Software without
  restriction, including without limitation the rights to use,
  copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following
  conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.
*/


/*
 * JMess.cpp
 */

#include "JMess.h"
#include "jacktrip_globals.h"
#include <QDebug>


//-------------------------------------------------------------------------------
/*! \brief Constructs a JMess object that has a jack client.
 *
 */
//-------------------------------------------------------------------------------
JMess::JMess()
{
    //Open a client connection to the JACK server.  Starting a
    //new server only to list its ports seems pointless, so we
    //specify JackNoStartServer.
    mClient = jack_client_open ("lsp", JackNoStartServer, &mStatus);
    if (mClient == NULL) {
        if (mStatus & JackServerFailed) {
            cerr << "JACK server not running" << endl;
        } else {
            cerr << "jack_client_open() failed, "
                 << "status = 0x%2.0x\n" << mStatus << endl;
        }
        exit(1);
    }
}


//-------------------------------------------------------------------------------
/*! \brief Distructor closes the jmess jack audio client.
 *
 */
//-------------------------------------------------------------------------------
JMess::~JMess()
{
    if (jack_client_close(mClient))
        cerr << "ERROR: Could not close the hidden jmess jack client." << endl;
}


//-------------------------------------------------------------------------------
/*! \brief Write an XML file with the name specified at xmlOutFile.
 *
 */
//-------------------------------------------------------------------------------
void JMess::writeOutput(QString xmlOutFile)
{
    //  QDomDocument jmess_xml;   QDomElement root;
    //  QDomElement connection;   QDomElement output;
    //  QDomElement input;        QDomText output_name;
    //  QDomText input_name;

    //  QVector<QString> OutputInput(2);

    //  this->setConnectedPorts();

    //  root = jmess_xml.createElement("jmess");
    //  for (QVector<QVector<QString> >::iterator it = mConnectedPorts.begin();
    //       it != mConnectedPorts.end(); ++it) {
    //    OutputInput = *it;
    //    //cout << "Output ===> " <<qPrintable(OutputInput[0]) << endl;
    //    //cout << "Input ===> " <<qPrintable(OutputInput[1]) << endl;

    //    //Initialize XML elements
    //    connection = jmess_xml.createElement("connection");
    //    output = jmess_xml.createElement("output");
    //    input = jmess_xml.createElement("input");
    //    output_name = jmess_xml.createTextNode(OutputInput[0]);
    //    input_name = jmess_xml.createTextNode(OutputInput[1]);

    //    jmess_xml.appendChild(root);      root.appendChild(connection);
    //    connection.appendChild(output);   connection.appendChild(input);
    //    output.appendChild(output_name);  input.appendChild(input_name);
    //  }

    //  //Write output file
    //  QFile file(xmlOutFile);
    //  string answer = "";
    //  //Check for existing file first, and confirm before overwriting
    //  if (file.exists()) {
    //    while ((answer != "yes") && (answer != "no")) {
    //      cout << "WARNING: The File " <<qPrintable(xmlOutFile)
    //	   << " exists. Do you want to overwrite it? (yes/no): ";
    //      cin >> answer;
    //    }
    //  }
    //  else {
    //    answer = "yes";
    //  }

    //  if (answer == "yes") {
    //    if (!file.open(QIODevice::WriteOnly)) {
    //      cerr << "Cannot open file for writing: "
    //	   << qPrintable(file.errorString()) << endl;
    //      exit(1);
    //    }

    //    QTextStream out(&file);
    //    jmess_xml.save(out, Indent);
    //    cout << qPrintable(xmlOutFile) << " written." << endl;
    //  }
}


//-------------------------------------------------------------------------------
/*! \brief Set list of ouput ports that have connections.
 *
 */
//-------------------------------------------------------------------------------
void JMess::setConnectedPorts()
{
    mConnectedPorts.clear();

    const char **ports, **connections; //vector of ports and connections
    QVector<QString> OutputInput(2); //helper variable

    //Get active output ports.
    ports = jack_get_ports (mClient, NULL, NULL, JackPortIsOutput);

    for (unsigned int out_i = 0; ports[out_i]; ++out_i) {
        if ((connections = jack_port_get_all_connections
             (mClient, jack_port_by_name(mClient, ports[out_i]))) != 0) {
            for (unsigned int in_i = 0; connections[in_i]; ++in_i) {
                OutputInput[0] = ports[out_i];
                //    cout << "Output ===> " <<qPrintable(OutputInput[0]) << endl;
                OutputInput[1] = connections[in_i];
                //    cout << "Input ===> " << qPrintable(OutputInput[1]) << endl;
                mConnectedPorts.append(OutputInput);
            }
        }
    }

    free(ports);
}
//*******************************************************************************
void JMess::connectSpawnedPorts(int nChans)
// called from UdpMasterListener::connectMesh
{
    int LAIRS[gMAX_WAIRS];
    int ctr = 0;
    for (int i = 0; i<gMAX_WAIRS; i++) LAIRS[i] = -1;
    QString bogus("badNumberField");

    const char **ports, **connections; //vector of ports and connections
    QVector<QString> OutputInput(2); //helper variable

    //Get active output ports.
    ports = jack_get_ports (mClient, NULL, NULL, JackPortIsOutput);

    int numberField = QString(WAIR_AUDIO_NAME).size();
    for (unsigned int out_i = 0; ports[out_i]; ++out_i) {
        bool tmp = QString(ports[out_i]).contains(WAIR_AUDIO_NAME);
        QChar c = QString(ports[out_i]).at(numberField);
        QString s = (c.isDigit())?QString(c):bogus;
        if((s!=bogus) && (s.toInt()<(gMAX_WAIRS-1)))
        {
            bool newOne = true;
            for (int i = 0; i<ctr; i++) if (newOne && (LAIRS[i]==s.toInt())) newOne = false;
            if (newOne)
            {
                LAIRS[ctr] = s.toInt();
                ctr++;
                qDebug() << ports[out_i] << tmp << s;
            }
        }
    }
    for (int i = 0; i<gMAX_WAIRS; i++) qDebug() << i << LAIRS[i]; // list connected LAIR IDs
    qDebug() << "---------------------------------";
    disconnectAll();
    //////////////////////
    //    // from hubLogger connects client to itself
    //    for (int i = 0; i<ctr; i++)
    //        {
    //            int k = i; // (j+(i+1))%ctr;
    //////////////////////////////////////////////////////////////////
#define CONNECT_CLIENT_TO_SELF

    for (int i = 0; i<ctr; i++)
#ifdef CONNECT_CLIENT_TO_SELF
#else
        for (int j = 0; j<(ctr-1); j++)
#endif
        {
#ifdef CONNECT_CLIENT_TO_SELF
            int k = i;
#else
            int k = (j+(i+1))%ctr;
#endif
            for (int l = 1; l<=nChans; l++) // chans are 1-based
            {
                qDebug() << "connect LAIR" << LAIRS[i] << ":receive_ " << l
                         <<"with LAIR" << LAIRS[k] << "send_" << l;

                QString left = (QString(WAIR_AUDIO_NAME + QString::number(LAIRS[i]) +
                                        ":receive_" + QString::number(l)));
                QString right = (QString(WAIR_AUDIO_NAME + QString::number(LAIRS[k]) +
                                         ":send_" + QString::number(l)));

                if (0 !=
                        jack_connect(mClient, left.toStdString().c_str(), right.toStdString().c_str())) {
                    qDebug() << "WARNING: port: " << left
                             << "and port: " << right
                             << " could not be connected.";
                }

            }
        }

    free(ports);
}




//-------------------------------------------------------------------------------
/*! \brief Disconnect all the clients.
 *
 */
//-------------------------------------------------------------------------------
void JMess::disconnectAll()
{
    QVector<QString> OutputInput(2);

    this->setConnectedPorts();

    for (QVector<QVector<QString> >::iterator it = mConnectedPorts.begin();
         it != mConnectedPorts.end(); ++it) {
        OutputInput = *it;

        if (jack_disconnect(mClient, OutputInput[0].toLatin1(), OutputInput[1].toLatin1())) {
            cerr << "WARNING: port: " << qPrintable(OutputInput[0])
                    << "and port: " << qPrintable(OutputInput[1])
                    << " could not be disconnected.\n";
        }
    }

}


//-------------------------------------------------------------------------------
/*! \brief Parse the XML input file.
 *
 * Returns 0 on success, or 1 if the file has an incorrect format or cannot
 * read the file.
 */
//-------------------------------------------------------------------------------
int JMess::parseXML(QString xmlInFile)
{
    //  mPortsToConnect.clear();
    //  QString errorStr;
    //  int errorLine;
    //  int errorColumn;

    //  QFile file(xmlInFile);
    //  if (!file.open(QIODevice::ReadOnly)) {
    //    cerr << "Cannot open file for reading: "
    //	 << qPrintable(file.errorString()) << endl;
    //    return 1;
    //  }

    //  QDomDocument doc;
    //  if (!doc.setContent(&file, true, &errorStr, &errorLine,
    //		      &errorColumn)) {
    //    cerr << "===================================================\n"
    //	 << "Error parsing XML input file:\n"
    //	 << "Parse error at line " << errorLine
    //	 << ", column " << errorColumn << "\n"
    //	 << qPrintable(errorStr) << "\n"
    //	 << "===================================================\n";
    //    return 1;
    //  }

    //  QDomElement jmess = doc.documentElement();
    //  if (jmess.tagName() != "jmess") {
    //    cerr << "Error: Root tag should be <jmess>: "
    //	 << qPrintable(jmess.tagName()) << endl;
    //    return 1;
    //  }


    //  QVector<QString> OutputInput(2);
    //  //First check for <connection> tag
    //  for(QDomNode n_cntn = jmess.firstChild();
    //      !n_cntn.isNull(); n_cntn = n_cntn.nextSibling()) {
    //    QDomElement cntn = n_cntn.toElement();
    //    if (cntn.tagName() == "connection") {
    //      //Now check for ouput & input tag
    //      for(QDomNode n_sck = cntn.firstChild();
    //	  !n_sck.isNull(); n_sck = n_sck.nextSibling()) {
    //	QDomElement sck = n_sck.toElement();
    //	//cout << qPrintable(sck.tagName()) << endl;
    //	//cout << qPrintable(sck.text()) << endl;
    //	if (sck.tagName() == "output") {
    //	  OutputInput[0] = sck.text();
    //	}
    //	else if (sck.tagName() == "input") {
    //	  OutputInput[1] = sck.text();
    //	}
    //      }
    //      mPortsToConnect.append(OutputInput);
    //    }
    //  }

    return 0;

}


//-------------------------------------------------------------------------------
/*! \brief Connect ports specified in input XML file xmlInFile
 *
 */
//-------------------------------------------------------------------------------
void JMess::connectPorts(QString xmlInFile)
{
    QVector<QString> OutputInput(2);

    //  if ( !(this->parseXML(xmlInFile)) ) {
    //    for (QVector<QVector<QString> >::iterator it = mPortsToConnect.begin();
    //	 it != mPortsToConnect.end(); ++it) {
    //      OutputInput = *it;

    //      if (jack_connect(mClient, OutputInput[0].toLatin1(), OutputInput[1].toLatin1())) {
    //	//Display a warining only if the error is not because the ports are already
    //	//connected, in case the program doesn't display anyting.
    //	if (EEXIST !=
    //        jack_connect(mClient, OutputInput[0].toLatin1(), OutputInput[1].toLatin1())) {
    //	  cerr << "WARNING: port: " << qPrintable(OutputInput[0])
    //	       << "and port: " << qPrintable(OutputInput[1])
    //	       << " could not be connected.\n";
    //	}
    //      }
    //    }
    //  }

}
