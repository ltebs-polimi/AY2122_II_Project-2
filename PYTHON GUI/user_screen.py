from pickle import TRUE
import sys

import time

import logging
from tkinter.font import BOLD

from PyQt5 import QtCore
from PyQt5.QtCore import (
    QObject,
    QThreadPool, 
    QRunnable, 
    pyqtSignal, 
    pyqtSlot
)

from PyQt5.QtWidgets import (
    QApplication,
    QMainWindow,
    QPushButton,
    QComboBox,
    QHBoxLayout,
    QWidget,
)

from PyQt5 import QtCore, QtGui, QtWidgets

import serial
import serial.tools.list_ports

# We import library dedicated to data plot
import pyqtgraph as pg
from pyqtgraph import PlotWidget

serial_ports_array = []

# Globals
CONN_STATUS = False
FINISHED_AMP = False
stringa_prova_AMP=''
PACKET_ARRIVED_AMP = False
result=int(0)

# Logging config
logging.basicConfig(format="%(message)s", level=logging.INFO)



#########################
# SERIAL_WORKER_SIGNALS #
#########################
class SerialWorkerSignals(QObject):
    """!
    @brief Class that defines the signals available to a serialworker.

    Available signals (with respective inputs) are:
        - device_port:
            str --> port name to which a device is connected
        - status:
            str --> port name
            int --> macro representing the state (0 - error during opening, 1 - success)
    """
    device_port = pyqtSignal(str)
    status = pyqtSignal(str, int)


#################
# SERIAL_WORKER #
#################
class SerialWorker(QRunnable):
    """!
    @brief Main class for serial communication: handles connection with device.
    """
    def __init__(self, serial_port_name):
        """!
        @brief Init worker.
        """
        self.is_killed = False
        super().__init__()
        # init port, params and signals
        self.port = serial.Serial()
        self.port_name = serial_port_name
        self.baudrate = 9600 # hard coded but can be a global variable, or an input param
        self.signals = SerialWorkerSignals()

    @pyqtSlot()
    def run(self):
        """!
        @brief Estabilish connection with desired serial port.
        """
        global CONN_STATUS
        global port_name_global

        if not CONN_STATUS:
            try:
                self.port = serial.Serial(port=self.port_name, baudrate=self.baudrate,
                                        write_timeout=0, timeout=2)    

                if self.port.is_open:

                    self.send('A')
                    time.sleep(0.1)
                    self.send('z')
                    time.sleep(0.1)

                    if (self.read() == "Glucose $$$"):
                        CONN_STATUS = True
                        self.signals.status.emit(self.port_name, 1)
                        port_name_global=self.port_name
                        time.sleep(1)


                    else:
                        self.is_killed = True
                        self.killed()


            except serial.SerialException:
                logging.info("Error with port {}.".format(self.port_name))
                self.signals.status.emit(self.port_name, 0)
                time.sleep(0.01)

       

    @pyqtSlot()
    def send(self, char):
        """!
        @brief Basic function to send a single char on serial port.
        """
        try:

            self.port.write(char.encode('utf-8'))
            logging.info("Written {} on port {}.".format(char, self.port_name))
        except:
            logging.info("Could not write {} on port {}.".format(char, self.port_name))


    @pyqtSlot()
    def read(self):
        """!
        @brief Basic function to send a single char on serial port.
        """
        stringa_test=''

        try:
            while(self.port.in_waiting>0):
                stringa_test+=self.port.read().decode('utf-8', errors='replace')
                logging.info("Received: {}".format(stringa_test))
                logging.info( self.port.in_waiting )

            return stringa_test
        except:
            logging.info("Could not receive {} on port {}.".format(stringa_test, self.port_name))


   
    @pyqtSlot()
    def killed(self):
        """!
        @brief Close the serial port before closing the app.
        """
        global CONN_STATUS
        if self.is_killed:
            self.port.close()
            time.sleep(0.01)
            CONN_STATUS = False
            self.signals.device_port.emit(self.port_name)

        logging.info("Killing the process")


#########################
# GRAPH_WORKER_SIGNALS #
#########################
class UpdateGraphSignals(QObject):
    """!
    @brief Class that defines the signals available to a UpdateGraphworker.

    Available signals (with respective inputs) are:
        - plot_values:
            int --> x value
            int --> y value
    """
    glucose_value = pyqtSignal(int)


######################
# UPDATE GRAPH WORKER#
######################
class UpdateGraphWorker(QRunnable):
    """!
    @brief Main class for serial communication: handles connection with device.
    """
    def __init__(self, port_name):
        """!
        @brief Init worker.
        """
        global GRAPH_STATUS
        self.is_killed_graph = False

        super().__init__()

        self.port_graph = serial.Serial()
        self.port_graph_name = port_name
        self.baudrate_graph = 9600 # hard coded but can be a global variable, or an input param

        self.port_graph = serial.Serial(port=self.port_graph_name, baudrate=self.baudrate_graph,
                                write_timeout=0, timeout=2)  
        

        logging.info("in update graph worker init")

        self.graph_signals = UpdateGraphSignals()


    @pyqtSlot()
    def run(self):
        """!
        @brief Estabilish connection with desired serial port.
        """
        global FINISHED_AMP

        while FINISHED_AMP == False:

            self.read_values()




    @pyqtSlot()
    def read_values(self):
        
        global FINISHED_AMP
        global stringa_prova_AMP
        global PACKET_ARRIVED_AMP
        global result
        stringa_result=''

        if(self.port_graph.in_waiting>0):
            stringa_prova_AMP += self.port_graph.read().decode('utf-8', errors='replace')
            logging.info(stringa_prova_AMP)
            PACKET_ARRIVED_AMP=True

        if PACKET_ARRIVED_AMP == True and stringa_prova_AMP[len(stringa_prova_AMP)-1]=="F":
            for i in range(1, len(stringa_prova_AMP)-1):
                stringa_result+=stringa_prova_AMP[i]
            
            result=int(stringa_result)
            self.graph_signals.glucose_value.emit(result)
            FINISHED_AMP=True
    

    @pyqtSlot()
    def killed_graph(self):
        """!
        @brief Close the serial port before closing the app.
        """
        global GRAPH_STATUS
        if self.is_killed_graph:
            self.port_graph.close()
            time.sleep(0.01)
            GRAPH_STATUS = False

        logging.info("Killing the process")




class Ui_UserWindow(object):
    def setupUi(self, UserWindow):

        self.serial_worker = SerialWorker(None)

        # create thread handler
        self.threadpool = QThreadPool()

        self.connected = CONN_STATUS

        self.CHECK_TOGGLE= bool(True)

        # Some random data
        self.hour = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
        self.temperature1 = [30, 32, 34, 32, 33, 31, 29, 32, 35, 45]
        self.temperature2 = [16, 20, 17, 23, 30, 25, 28, 26, 22, 32]


        UserWindow.setObjectName("UserWindow")
        UserWindow.resize(1124, 721)
        self.centralwidget = QtWidgets.QWidget(UserWindow)
        self.centralwidget.setObjectName("centralwidget")
        self.gridLayout_3 = QtWidgets.QGridLayout(self.centralwidget)
        self.gridLayout_3.setObjectName("gridLayout_3")
        self.frame = QtWidgets.QFrame(self.centralwidget)
        self.frame.setFrameShape(QtWidgets.QFrame.StyledPanel)
        self.frame.setFrameShadow(QtWidgets.QFrame.Raised)
        self.frame.setObjectName("frame")
        self.gridLayout_2 = QtWidgets.QGridLayout(self.frame)
        self.gridLayout_2.setObjectName("gridLayout_2")
        self.gridLayout = QtWidgets.QGridLayout()
        self.gridLayout.setObjectName("gridLayout")
        spacerItem = QtWidgets.QSpacerItem(20, 40, QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding)
        self.gridLayout.addItem(spacerItem, 2, 1, 1, 1)
        spacerItem1 = QtWidgets.QSpacerItem(20, 40, QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding)
        self.gridLayout.addItem(spacerItem1, 4, 1, 1, 1)
        self.Glucose_label = QtWidgets.QLabel(self.frame)
        self.Glucose_label.setAlignment(QtCore.Qt.AlignCenter)
        self.Glucose_label.setObjectName("Glucose_label")
        self.Glucose_label.setFont(QtGui.QFont('Arial', 20))
        self.gridLayout.addWidget(self.Glucose_label, 0, 1, 1, 1)
        self.Value_label = QtWidgets.QLabel(self.frame)
        self.Value_label.setTextFormat(QtCore.Qt.AutoText)
        self.Value_label.setAlignment(QtCore.Qt.AlignCenter)
        self.Value_label.setObjectName("Value_label")
        self.gridLayout.addWidget(self.Value_label, 1, 1, 1, 1)
        self.Start_button = QtWidgets.QPushButton(self.frame)
        self.Start_button.setObjectName("Start_button")
        self.gridLayout.addWidget(self.Start_button, 3, 1, 1, 1)
        spacerItem2 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.gridLayout.addItem(spacerItem2, 3, 0, 1, 1)
        spacerItem3 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.gridLayout.addItem(spacerItem3, 3, 2, 1, 1)
        self.horizontalLayout_2 = QtWidgets.QHBoxLayout()
        self.horizontalLayout_2.setContentsMargins(40, -1, 40, -1)
        self.horizontalLayout_2.setSpacing(200)
        self.horizontalLayout_2.setObjectName("horizontalLayout_2")

        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)


        self.pushButton = QtWidgets.QPushButton(self.frame, clicked = lambda: self.on_toggle(self.CHECK_TOGGLE))
        self.pushButton.setMinimumSize(QtCore.QSize(150, 28))
        self.pushButton.setObjectName("pushButton")
        font = QtGui.QFont()
        font.setFamily("Yu Gothic UI Semibold")
        font.setBold(True)
        self.pushButton.setFont(font)   
        self.horizontalLayout_2.addWidget(self.pushButton)
        self.label_3 = QtWidgets.QLabel(self.frame)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.label_3.sizePolicy().hasHeightForWidth())
        self.label_3.setSizePolicy(sizePolicy)
        self.label_3.setMinimumSize(QtCore.QSize(150, 28))
        self.label_3.setStyleSheet("background-color: rgb(255, 0, 0);")
        self.label_3.setAlignment(QtCore.Qt.AlignCenter)
        self.label_3.setObjectName("label_3")
        font = QtGui.QFont()
        font.setFamily("Yu Gothic UI Semibold")
        font.setBold(True)
        self.label_3.setFont(font)        
        self.horizontalLayout_2.addWidget(self.label_3)
        self.gridLayout.addLayout(self.horizontalLayout_2, 5, 0, 1, 3)
        self.gridLayout.setColumnStretch(0, 1)
        self.gridLayout.setColumnStretch(1, 1)
        self.gridLayout.setColumnStretch(2, 1)
        self.gridLayout.setRowStretch(0, 3)
        self.gridLayout.setRowStretch(1, 3)
        self.gridLayout.setRowStretch(2, 1)
        self.gridLayout.setRowStretch(3, 1)
        self.gridLayout.setRowStretch(4, 1)
        self.gridLayout.setRowStretch(5, 1)
        self.gridLayout_2.addLayout(self.gridLayout, 0, 0, 1, 1)
        self.gridLayout_3.addWidget(self.frame, 0, 0, 1, 1)
        UserWindow.setCentralWidget(self.centralwidget)

        self.retranslateUi(UserWindow)
        QtCore.QMetaObject.connectSlotsByName(UserWindow)

        self.serialscan()

    def retranslateUi(self, UserWindow):
        _translate = QtCore.QCoreApplication.translate
        UserWindow.setWindowTitle(_translate("UserWindow", "User Screen"))
        self.Glucose_label.setText(_translate("UserWindow", "GLUCOSE CONCENTRATION (mg/dL)"))
        self.Value_label.setText(_translate("UserWindow", "VALUE"))
        self.Start_button.setText(_translate("UserWindow", "START MEASUREMENT"))
        self.pushButton.setText(_translate("UserWindow", ""))
        self.label_3.setText(_translate("UserWindow", "NOT CONNECTED"))
        self.connect_to_COM()


    def connect_to_COM(self):

        #scan continuously the serial port
        serial_ports_array = self.serialscan()

        i=0

        for i in range(len(serial_ports_array)):
            # setup reading worker
            self.serial_worker = SerialWorker(serial_ports_array[i]) # needs to be re defined
            # connect worker signals to functions
            self.serial_worker.signals.status.connect(self.check_serialport_status)
            self.serial_worker.signals.device_port.connect(self.connected_device)
            # execute the worker
            self.threadpool.start(self.serial_worker)

    def show_result(self):
        
        self.Value_label.setFont(QtGui.QFont('Arial', 80))
        self.Value_label.setText(str(result))



    ####################
    # SERIAL INTERFACE #
    ####################
    def serialscan(self):
        """!
        @brief Scans all serial ports and create a list.
        """

        self.port_text = ""


        # acquire list of serial ports 
        serial_ports = [
                p.name
                for p in serial.tools.list_ports.comports()
            ]

        return serial_ports

    ##################
    # SERIAL SIGNALS #
    ##################

    @pyqtSlot()
    def Start_AMP(self):
        """!
        @brief Draw the plots.
        """
        global FINISHED_AMP

        FINISHED_AMP= False

        self.serial_worker.send('L')
        time.sleep(0.1)
        self.serial_worker.send('z')
        time.sleep(0.1)

        self.serial_worker.is_killed = True
        self.serial_worker.killed()
        time.sleep(0.5)

        self.graph_worker = UpdateGraphWorker(port_name_global) # needs to be re defined
        # connect worker signals to functions
        self.graph_worker.graph_signals.glucose_value.connect(self.show_result)

        # execute the worker
        self.threadpool.start(self.graph_worker)


    @pyqtSlot(bool)
    def on_toggle(self, checked):
        """!
        @brief Allow connection and disconnection from selected serial port.
        """
        if checked:
            
            if CONN_STATUS:
                # kill serial thread
                self.label_3.setStyleSheet("background-color:rgb(255,0,0);")
                self.label_3.setText("NOT CONNECTED")
                self.serial_worker.is_killed = True
                self.serial_worker.killed()
                self.label_3.setDisabled(False) 
                self.pushButton.setText(
                    "Connect to port {}".format(self.port_text)
                )
                self.CHECK_TOGGLE = bool(False)

        else:

            self.connect_to_COM()

            self.CHECK_TOGGLE = bool(True)





    def check_serialport_status(self, port_name, status):
        """!
        @brief Handle the status of the serial port connection.

        Available status:
            - 0  --> Error during opening of serial port
            - 1  --> Serial port opened correctly
        """
        if status == 0:
            self.pushButton.setChecked(False)
        elif status == 1:
            # enable all the widgets on the interface

            self.pushButton.setText(
                "Disconnect from port {}".format(port_name)
            )

            self.label_3.setStyleSheet("background-color:rgb(0,255,0);")
            self.label_3.setText("CONNECTED")

    def connected_device(self, port_name):
        """!
        @brief Checks on the termination of the serial worker.
        """
        logging.info("Port {} closed.".format(port_name))


    def ExitHandler(self):
        """!
        @brief Kill every possible running thread upon exiting application.
        """
        self.serial_worker.is_killed = True
        self.serial_worker.killed()


#############
#  RUN APP  #
#############


if __name__ == "__main__":
    import sys
    app = QtWidgets.QApplication(sys.argv)
    UserWindow = QtWidgets.QMainWindow()
    ui = Ui_UserWindow()
    ui.setupUi(UserWindow)
    UserWindow.show()
    sys.exit(app.exec_())
