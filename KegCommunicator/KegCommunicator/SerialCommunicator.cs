using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace KegCommunicator {
    class SerialCommunicator {

        private SerialPort serialPort;
        private int delay = 1000;

        public SerialCommunicator(string portName, int baud) {
            serialPort = new SerialPort();
            serialPort.BaudRate = baud;
            serialPort.PortName = portName;
        }

        public string send_command(string cmd) {
            // Open the serial port
            serialPort.Open();

            // Send command
            serialPort.Write(cmd);

            // Wait for response
            Thread.Sleep(delay);

            // Look for/read a response
            int bytes = serialPort.BytesToRead;
            string response = "Success";
            if (bytes != 0) {
                serialPort.ReadLine();
            }

            // Close the serial port
            serialPort.Close();

            // Return response
            return response;
        }
    }
}
