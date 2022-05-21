from pickle import TRUE
import sys

import time

import logging

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
from PyQt5.QtSerialPort import QSerialPort, QSerialPortInfo

import serial
import serial.tools.list_ports

# We import library dedicated to data plot
import pyqtgraph as pg
from pyqtgraph import PlotWidget, plot
import pyqtgraph as pg
import os
from random import randint

# Globals
CONN_STATUS = False
UPDATE_SCAN_FLAG=False 
UPDATE_TIME_FLAG=False 
RECEIVE_CV_DATA= False
FINISHED_CV_GRAPH = False
PACKET_ARRIVED = False
READ_PACKET_DATA= False
LIMIT_REACHED= False

UPDATE_SCAN = "B"
UPDATE_INITIAL = "C"
UPDATE_FINAL= "D"
UPDATE_TIME= "E"


port_name_global = 0 
current_CV = int(0)
potential_CV = int(0)
current_CV_H = int(0)
current_CV_L = int(0)
potential_CV_H= int(0)
potential_CV_L= int(0)
stringa_prova=''


# Global variables
final_value=int()
initial_value=int()
time_value=int()
scan_rate=int()

serial_ports_array = []

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
    plot_values = pyqtSignal(int, int)
    data_ready_signal = pyqtSignal(object)





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

        global FINISHED_CV_GRAPH
        global potential_CV
        global LIMIT_REACHED
        global RECEIVE_CV_DATA
        global initial_value
        global final_value
        global READ_PACKET_DATA

     
        
        if RECEIVE_CV_DATA:
            
            while FINISHED_CV_GRAPH == False:

                self.read_CV_plot()
    


    @pyqtSlot()
    def read_CV_plot(self):
        """!
        @brief Basic function to send a single char on serial port.
        """
        global FINISHED_CV_GRAPH
        global RECEIVE_CV_DATA
        global current_CV_H
        global current_CV_L
        global PACKET_ARRIVED 
        global READ_PACKET_DATA
        global potential_CV
        global LIMIT_REACHED
        global initial_value
        global final_value
        global potential_CV_H
        global potential_CV_L
        global current_CV
        global stringa_prova
        stringa_current_H=''
        stringa_current_L=''
        stringa_potential_H=''
        stringa_potential_L=''
        A_index=0
        b_index=0
        c_index=0
        d_index=0
        z_index=0

        PACKET_ARRIVED=False
        READ_PACKET_DATA=False

        if RECEIVE_CV_DATA:

            if(self.port_graph.in_waiting>0):
                stringa_prova += self.port_graph.read().decode('utf-8', errors='replace')
                logging.info(stringa_prova)
                PACKET_ARRIVED=True

            if PACKET_ARRIVED == True and stringa_prova[0] == 'A' and stringa_prova[len(stringa_prova)-1] == 'z':
                logging.info("packet_arrived")
                logging.info(len(stringa_prova))               
                for i in range(len(stringa_prova)):
                    if(stringa_prova[i])=='A':
                        A_index=i

                    if(stringa_prova[i])=='b':
                        b_index=i

                    if(stringa_prova[i])=='c':
                        c_index=i

                    if(stringa_prova[i])=='d':
                        d_index=i

                    if(stringa_prova[i])=='z':
                        z_index=i


                logging.info(A_index)
                logging.info(b_index)
                logging.info(c_index)
                logging.info(d_index)
                logging.info(z_index)

                stringa_current_H = stringa_prova[A_index+1:b_index]
                stringa_current_L= stringa_prova[b_index+1:c_index]
                stringa_potential_H= stringa_prova[c_index+1:d_index]
                stringa_potential_L= stringa_prova[d_index+1:z_index]

                logging.info(stringa_current_H)
                logging.info(stringa_current_L)
                logging.info(stringa_potential_H)
                logging.info(stringa_potential_L)
            
                PACKET_ARRIVED = False
                READ_PACKET_DATA = True


            if(READ_PACKET_DATA == True):
                current_CV = (int(stringa_current_H) << 8) | (int(stringa_current_L))
                potential_CV = (int(stringa_potential_H) << 8) | (int(stringa_potential_L))

                logging.info(current_CV)
                logging.info(potential_CV)

                READ_PACKET_DATA= False

                self.graph_signals.plot_values.emit(potential_CV, current_CV) 

                stringa_prova=''
                stringa_current_H=''
                stringa_current_L=''
                stringa_potential_H=''
                stringa_potential_L=''
                A_index=0
                b_index=0
                c_index=0
                d_index=0
                z_index=0

                self.port_graph.reset_input_buffer()


            if(potential_CV == final_value):
                LIMIT_REACHED = True

            if(potential_CV == initial_value and LIMIT_REACHED):
                FINISHED_CV_GRAPH = True

    @pyqtSlot()
    def send_graph(self, char):
        """!
        @brief Basic function to send a single char on serial port.
        """
        try:

            self.port_graph.write(char.encode('utf-8'))
            logging.info("Written {} on port {}.".format(char, port_name_global))
        except:
            logging.info("Could not write {} on port {}.".format(char, port_name_global))



class Ui_ClinicianWindow(object):
    def setupUi(self, ClinicianWindow):

        self.serial_worker = SerialWorker(None)

        # create thread handler
        self.threadpool = QThreadPool()

        self.connected = CONN_STATUS

        self.CHECK_TOGGLE= bool(True)

        # Some random data
        self.x = [] 
        self.y = []  



        ClinicianWindow.setObjectName("ClinicianWindow")
        ClinicianWindow.resize(1064, 735)
        self.centralwidget = QtWidgets.QWidget(ClinicianWindow)
        self.centralwidget.setObjectName("centralwidget")
        self.gridLayout = QtWidgets.QGridLayout(self.centralwidget)
        self.gridLayout.setObjectName("gridLayout")
        self.frame = QtWidgets.QFrame(self.centralwidget)
        self.frame.setStyleSheet("")
        self.frame.setFrameShape(QtWidgets.QFrame.StyledPanel)
        self.frame.setFrameShadow(QtWidgets.QFrame.Raised)
        self.frame.setObjectName("frame")
        self.gridLayout_3 = QtWidgets.QGridLayout(self.frame)
        self.gridLayout_3.setObjectName("gridLayout_3")
        self.gridLayout_2 = QtWidgets.QGridLayout()
        self.gridLayout_2.setObjectName("gridLayout_2")
        self.horizontalLayout_3 = QtWidgets.QHBoxLayout()
        self.horizontalLayout_3.setContentsMargins(40, 10, 40, -1)
        self.horizontalLayout_3.setSpacing(200)
        self.horizontalLayout_3.setObjectName("horizontalLayout_3")
        self.Connection_port_label = QtWidgets.QLabel(self.frame)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.Connection_port_label.sizePolicy().hasHeightForWidth())
        self.Connection_port_label.setSizePolicy(sizePolicy)
        self.Connection_port_label.setMinimumSize(QtCore.QSize(150, 28))
        self.Connection_port_label.setMaximumSize(QtCore.QSize(16777215, 16777215))
        self.Connection_port_label.setObjectName("Connection_port_label")
        self.horizontalLayout_3.addWidget(self.Connection_port_label)
        self.Connection_button = QtWidgets.QPushButton(self.frame,  clicked = lambda: self.on_toggle(self.CHECK_TOGGLE))
        self.Connection_button.setMinimumSize(QtCore.QSize(150, 28))
        font = QtGui.QFont()
        font.setFamily("Yu Gothic UI Semibold")
        font.setBold(True)
        font.setWeight(75)
        self.Connection_button.setFont(font)
        self.Connection_button.setObjectName("Connection_button")
        self.horizontalLayout_3.addWidget(self.Connection_button)
        self.Connection_label = QtWidgets.QLabel(self.frame)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.Connection_label.sizePolicy().hasHeightForWidth())
        self.Connection_label.setSizePolicy(sizePolicy)
        self.Connection_label.setMinimumSize(QtCore.QSize(150, 28))
        self.Connection_label.setMaximumSize(QtCore.QSize(16777215, 16777215))
        font = QtGui.QFont()
        font.setFamily("Yu Gothic UI Semibold")
        font.setBold(True)
        font.setWeight(75)
        self.Connection_label.setFont(font)
        self.Connection_label.setFocusPolicy(QtCore.Qt.StrongFocus)
        self.Connection_label.setStyleSheet("background-color:rgb(255,0,0);")
        self.Connection_label.setAlignment(QtCore.Qt.AlignCenter)
        self.Connection_label.setObjectName("Connection_label")
        self.horizontalLayout_3.addWidget(self.Connection_label)
        self.gridLayout_2.addLayout(self.horizontalLayout_3, 1, 0, 1, 1)
        self.tabWidget = QtWidgets.QTabWidget(self.frame)
        font = QtGui.QFont()
        font.setFamily("Yu Gothic UI Semibold")
        font.setBold(True)
        font.setWeight(75)
        self.tabWidget.setFont(font)
        self.tabWidget.setObjectName("tabWidget")
        self.tab_3 = QtWidgets.QWidget()
        self.tab_3.setObjectName("tab_3")
        self.gridLayout_4 = QtWidgets.QGridLayout(self.tab_3)
        self.gridLayout_4.setSizeConstraint(QtWidgets.QLayout.SetDefaultConstraint)
        self.gridLayout_4.setObjectName("gridLayout_4")
        spacerItem = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.gridLayout_4.addItem(spacerItem, 0, 1, 1, 1)
        self.gridLayout_9 = QtWidgets.QGridLayout()
        self.gridLayout_9.setContentsMargins(-1, -1, -1, 0)
        self.gridLayout_9.setObjectName("gridLayout_9")
        self.Time_CV_label = QtWidgets.QLabel(self.tab_3)
        font = QtGui.QFont()
        font.setPointSize(10)
        self.Time_CV_label.setFont(font)
        self.Time_CV_label.setAlignment(QtCore.Qt.AlignCenter)
        self.Time_CV_label.setObjectName("Time_CV_label")
        self.gridLayout_9.addWidget(self.Time_CV_label, 9, 0, 1, 1)
        self.Initial_value_CV_label = QtWidgets.QLabel(self.tab_3)
        font = QtGui.QFont()
        font.setFamily("Yu Gothic UI Semibold")
        font.setPointSize(10)
        font.setBold(True)
        font.setWeight(75)
        self.Initial_value_CV_label.setFont(font)
        self.Initial_value_CV_label.setAlignment(QtCore.Qt.AlignCenter)
        self.Initial_value_CV_label.setObjectName("Initial_value_CV_label")
        self.gridLayout_9.addWidget(self.Initial_value_CV_label, 0, 0, 1, 1)
        self.Final_value_CV_label = QtWidgets.QLabel(self.tab_3)
        font = QtGui.QFont()
        font.setFamily("Yu Gothic UI Semibold")
        font.setPointSize(10)
        font.setBold(True)
        font.setWeight(75)
        self.Final_value_CV_label.setFont(font)
        self.Final_value_CV_label.setAlignment(QtCore.Qt.AlignCenter)
        self.Final_value_CV_label.setObjectName("Final_value_CV_label")
        self.gridLayout_9.addWidget(self.Final_value_CV_label, 3, 0, 1, 1)
        self.horizontalLayout_6 = QtWidgets.QHBoxLayout()
        self.horizontalLayout_6.setObjectName("horizontalLayout_6")
        self.Scan_CV_textEdit = QtWidgets.QTextEdit(self.tab_3)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.Scan_CV_textEdit.sizePolicy().hasHeightForWidth())
        self.Scan_CV_textEdit.setSizePolicy(sizePolicy)
        self.Scan_CV_textEdit.setMinimumSize(QtCore.QSize(93, 28))
        self.Scan_CV_textEdit.setObjectName("Scan_CV_textEdit")
        self.horizontalLayout_6.addWidget(self.Scan_CV_textEdit)
        self.Scan_update_button = QtWidgets.QPushButton(self.tab_3)
        self.Scan_update_button.setDisabled(True)
        font = QtGui.QFont()
        font.setFamily("Yu Gothic UI Semibold")
        font.setBold(True)
        font.setWeight(75)
        self.Scan_update_button.setFont(font)
        self.Scan_update_button.setObjectName("Scan_update_button")
        self.horizontalLayout_6.addWidget(self.Scan_update_button)
        self.horizontalLayout_6.setStretch(0, 1)
        self.horizontalLayout_6.setStretch(1, 1)
        self.gridLayout_9.addLayout(self.horizontalLayout_6, 7, 0, 1, 1)
        spacerItem1 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.gridLayout_9.addItem(spacerItem1, 5, 0, 1, 1)
        self.horizontalLayout_4 = QtWidgets.QHBoxLayout()
        self.horizontalLayout_4.setObjectName("horizontalLayout_4")
        self.Initial_CV_textEdit = QtWidgets.QTextEdit(self.tab_3)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.Initial_CV_textEdit.sizePolicy().hasHeightForWidth())
        self.Initial_CV_textEdit.setSizePolicy(sizePolicy)
        self.Initial_CV_textEdit.setMinimumSize(QtCore.QSize(93, 28))
        self.Initial_CV_textEdit.setMaximumSize(QtCore.QSize(16777215, 16777215))
        self.Initial_CV_textEdit.setObjectName("Initial_CV_textEdit")
        self.horizontalLayout_4.addWidget(self.Initial_CV_textEdit)
        self.Initial_update_button = QtWidgets.QPushButton(self.tab_3)
        self.Initial_update_button.setDisabled(True)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.Initial_update_button.sizePolicy().hasHeightForWidth())
        self.Initial_update_button.setSizePolicy(sizePolicy)
        font = QtGui.QFont()
        font.setFamily("Yu Gothic UI Semibold")
        font.setBold(True)
        font.setWeight(75)
        self.Initial_update_button.setFont(font)
        self.Initial_update_button.setObjectName("Initial_update_button")
        self.horizontalLayout_4.addWidget(self.Initial_update_button)
        self.horizontalLayout_4.setStretch(0, 1)
        self.horizontalLayout_4.setStretch(1, 1)
        self.gridLayout_9.addLayout(self.horizontalLayout_4, 1, 0, 1, 1)
        spacerItem2 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.gridLayout_9.addItem(spacerItem2, 8, 0, 1, 1)
        self.Scanrate_CV_label = QtWidgets.QLabel(self.tab_3)
        font = QtGui.QFont()
        font.setFamily("Yu Gothic UI Semibold")
        font.setPointSize(10)
        font.setBold(True)
        font.setWeight(75)
        self.Scanrate_CV_label.setFont(font)
        self.Scanrate_CV_label.setAlignment(QtCore.Qt.AlignCenter)
        self.Scanrate_CV_label.setObjectName("Scanrate_CV_label")
        self.gridLayout_9.addWidget(self.Scanrate_CV_label, 6, 0, 1, 1)
        self.horizontalLayout_5 = QtWidgets.QHBoxLayout()
        self.horizontalLayout_5.setObjectName("horizontalLayout_5")
        self.Final_CV_textEdit = QtWidgets.QTextEdit(self.tab_3)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.Final_CV_textEdit.sizePolicy().hasHeightForWidth())
        self.Final_CV_textEdit.setSizePolicy(sizePolicy)
        self.Final_CV_textEdit.setMinimumSize(QtCore.QSize(93, 28))
        self.Final_CV_textEdit.setObjectName("Final_CV_textEdit")
        self.horizontalLayout_5.addWidget(self.Final_CV_textEdit)
        self.Final_update_button = QtWidgets.QPushButton(self.tab_3)
        self.Final_update_button.setDisabled(True)
        font = QtGui.QFont()
        font.setFamily("Yu Gothic UI Semibold")
        font.setBold(True)
        font.setWeight(75)
        self.Final_update_button.setFont(font)
        self.Final_update_button.setObjectName("Final_update_button")
        self.horizontalLayout_5.addWidget(self.Final_update_button)
        self.horizontalLayout_5.setStretch(0, 1)
        self.horizontalLayout_5.setStretch(1, 1)
        self.gridLayout_9.addLayout(self.horizontalLayout_5, 4, 0, 1, 1)
        spacerItem3 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.gridLayout_9.addItem(spacerItem3, 2, 0, 1, 1)
        self.horizontalLayout = QtWidgets.QHBoxLayout()
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.Time_CV_textEdit = QtWidgets.QTextEdit(self.tab_3)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.Time_CV_textEdit.sizePolicy().hasHeightForWidth())
        self.Time_CV_textEdit.setSizePolicy(sizePolicy)
        self.Time_CV_textEdit.setMinimumSize(QtCore.QSize(93, 28))
        self.Time_CV_textEdit.setObjectName("Time_CV_textEdit")
        self.horizontalLayout.addWidget(self.Time_CV_textEdit)
        self.Time_update_button = QtWidgets.QPushButton(self.tab_3)
        self.Time_update_button.setDisabled(True)
        self.Time_update_button.setObjectName("Time_update_button")
        self.horizontalLayout.addWidget(self.Time_update_button)
        self.horizontalLayout.setStretch(0, 1)
        self.horizontalLayout.setStretch(1, 1)
        self.gridLayout_9.addLayout(self.horizontalLayout, 10, 0, 1, 1)
        self.gridLayout_9.setRowStretch(0, 1)
        self.gridLayout_9.setRowStretch(1, 1)
        self.gridLayout_9.setRowStretch(2, 1)
        self.gridLayout_9.setRowStretch(3, 1)
        self.gridLayout_9.setRowStretch(4, 1)
        self.gridLayout_9.setRowStretch(5, 1)
        self.gridLayout_9.setRowStretch(6, 1)
        self.gridLayout_9.setRowStretch(7, 1)
        self.gridLayout_9.setRowStretch(8, 1)
        self.gridLayout_9.setRowStretch(9, 1)
        self.gridLayout_9.setRowStretch(10, 1)
        self.gridLayout_4.addLayout(self.gridLayout_9, 0, 2, 2, 1)
        self.gridLayout_10 = QtWidgets.QGridLayout()
        self.gridLayout_10.setObjectName("gridLayout_10")

        # self.CV_graphicsView = QtWidgets.QGraphicsView(self.tab_3)
        # self.CV_graphicsView.setObjectName("CV_graphicsView")

        self.graphWidget_CV = PlotWidget(self.tab_3)
        self.graphWidget_CV.setObjectName("graphicsView")


        self.gridLayout_10.addWidget(self.graphWidget_CV, 3, 0, 1, 3)
        spacerItem4 = QtWidgets.QSpacerItem(20, 40, QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding)
        self.gridLayout_10.addItem(spacerItem4, 4, 0, 1, 1)
        spacerItem5 = QtWidgets.QSpacerItem(20, 40, QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding)
        self.gridLayout_10.addItem(spacerItem5, 2, 0, 1, 1)
        self.horizontalLayout_7 = QtWidgets.QHBoxLayout()
        self.horizontalLayout_7.setContentsMargins(120, 0, 120, 0)
        self.horizontalLayout_7.setSpacing(180)
        self.horizontalLayout_7.setObjectName("horizontalLayout_7")
        self.Draw_CV_button = QtWidgets.QPushButton(self.tab_3, clicked= lambda: self.draw_CV())
        self.Draw_CV_button.setDisabled(True)
        font = QtGui.QFont()
        font.setFamily("Yu Gothic UI Semibold")
        font.setBold(True)
        font.setWeight(75)
        self.Draw_CV_button.setFont(font)
        self.Draw_CV_button.setObjectName("Draw_CV_button")
        self.horizontalLayout_7.addWidget(self.Draw_CV_button)
        self.Clear_CV_button = QtWidgets.QPushButton(self.tab_3, clicked=self.graphWidget_CV.clear)
        font = QtGui.QFont()
        font.setFamily("Yu Gothic UI Semibold")
        font.setBold(True)
        font.setWeight(75)
        self.Clear_CV_button.setFont(font)
        self.Clear_CV_button.setObjectName("Clear_CV_button")
        self.horizontalLayout_7.addWidget(self.Clear_CV_button)
        self.gridLayout_10.addLayout(self.horizontalLayout_7, 5, 0, 1, 3)
        self.horizontalLayout_2 = QtWidgets.QHBoxLayout()
        self.horizontalLayout_2.setObjectName("horizontalLayout_2")
        spacerItem6 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout_2.addItem(spacerItem6)
        self.Ready_CV_label = QtWidgets.QLabel(self.tab_3)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.Ready_CV_label.sizePolicy().hasHeightForWidth())
        self.Ready_CV_label.setSizePolicy(sizePolicy)
        self.Ready_CV_label.setMinimumSize(QtCore.QSize(110, 28))
        self.Ready_CV_label.setStyleSheet("background-color: rgb(255, 0, 0);")
        self.Ready_CV_label.setAlignment(QtCore.Qt.AlignCenter)
        self.Ready_CV_label.setObjectName("Ready_CV_label")
        font = QtGui.QFont()
        font.setFamily("Yu Gothic UI Semibold")
        font.setBold(True)
        font.setWeight(75)
        self.Ready_CV_label.setFont(font)
        self.horizontalLayout_2.addWidget(self.Ready_CV_label)
        spacerItem7 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout_2.addItem(spacerItem7)
        self.horizontalLayout_2.setStretch(0, 1)
        self.horizontalLayout_2.setStretch(1, 1)
        self.horizontalLayout_2.setStretch(2, 1)
        self.gridLayout_10.addLayout(self.horizontalLayout_2, 1, 0, 1, 1)
        self.gridLayout_10.setRowStretch(0, 1)
        self.gridLayout_10.setRowStretch(1, 1)
        self.gridLayout_10.setRowStretch(2, 1)
        self.gridLayout_10.setRowStretch(3, 25)
        self.gridLayout_10.setRowStretch(4, 1)
        self.gridLayout_10.setRowStretch(5, 1)
        self.gridLayout_4.addLayout(self.gridLayout_10, 0, 0, 2, 1)
        spacerItem8 = QtWidgets.QSpacerItem(20, 40, QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding)
        self.gridLayout_4.addItem(spacerItem8, 2, 2, 1, 1)
        self.gridLayout_4.setColumnStretch(0, 15)
        self.gridLayout_4.setColumnStretch(2, 5)
        self.gridLayout_4.setRowStretch(0, 20)
        self.tabWidget.addTab(self.tab_3, "")
        self.tab_4 = QtWidgets.QWidget()
        self.tab_4.setObjectName("tab_4")
        self.gridLayout_6 = QtWidgets.QGridLayout(self.tab_4)
        self.gridLayout_6.setObjectName("gridLayout_6")
        self.gridLayout_7 = QtWidgets.QGridLayout()
        self.gridLayout_7.setObjectName("gridLayout_7")
        spacerItem9 = QtWidgets.QSpacerItem(20, 40, QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding)
        self.gridLayout_7.addItem(spacerItem9, 2, 1, 1, 4)
        spacerItem10 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.gridLayout_7.addItem(spacerItem10, 3, 5, 1, 1)
        spacerItem11 = QtWidgets.QSpacerItem(20, 40, QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Expanding)
        self.gridLayout_7.addItem(spacerItem11, 4, 1, 1, 4)
        spacerItem12 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.gridLayout_7.addItem(spacerItem12, 3, 0, 1, 1)
        spacerItem13 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.gridLayout_7.addItem(spacerItem13, 3, 3, 1, 1)
        self.Start_amp_button = QtWidgets.QPushButton(self.tab_4)
        self.Start_amp_button.setObjectName("Start_amp_button")
        self.gridLayout_7.addWidget(self.Start_amp_button, 3, 1, 1, 2)
        self.Stop_amp_button = QtWidgets.QPushButton(self.tab_4)
        self.Stop_amp_button.setObjectName("Stop_amp_button")
        self.gridLayout_7.addWidget(self.Stop_amp_button, 3, 4, 1, 1)
        self.Fetch_amp_button = QtWidgets.QPushButton(self.tab_4)
        self.Fetch_amp_button.setObjectName("Fetch_amp_button")
        self.gridLayout_7.addWidget(self.Fetch_amp_button, 0, 5, 1, 1)


        self.graphWidget_AMP = PlotWidget(self.tab_4)
        self.graphWidget_AMP.setObjectName("Amp_graphicsView")


        self.gridLayout_7.addWidget(self.graphWidget_AMP, 1, 0, 1, 6)
        self.gridLayout_7.setColumnStretch(0, 1)
        self.gridLayout_7.setColumnStretch(1, 1)
        self.gridLayout_7.setColumnStretch(3, 1)
        self.gridLayout_7.setColumnStretch(4, 1)
        self.gridLayout_7.setColumnStretch(5, 1)
        self.gridLayout_7.setRowStretch(0, 18)
        self.gridLayout_7.setRowStretch(1, 1)
        self.gridLayout_6.addLayout(self.gridLayout_7, 1, 0, 1, 1)
        self.tabWidget.addTab(self.tab_4, "")
        self.tab_5 = QtWidgets.QWidget()
        self.tab_5.setObjectName("tab_5")
        self.verticalLayout_3 = QtWidgets.QVBoxLayout(self.tab_5)
        self.verticalLayout_3.setObjectName("verticalLayout_3")
        self.frame_2 = QtWidgets.QFrame(self.tab_5)
        self.frame_2.setFrameShape(QtWidgets.QFrame.StyledPanel)
        self.frame_2.setFrameShadow(QtWidgets.QFrame.Raised)
        self.frame_2.setObjectName("frame_2")
        self.gridLayout_8 = QtWidgets.QGridLayout(self.frame_2)
        self.gridLayout_8.setObjectName("gridLayout_8")
        self.gridLayout_5 = QtWidgets.QGridLayout()
        self.gridLayout_5.setObjectName("gridLayout_5")
        spacerItem14 = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.gridLayout_5.addItem(spacerItem14, 0, 0, 1, 1)
        self.Value_data_label = QtWidgets.QLabel(self.frame_2)
        self.Value_data_label.setAlignment(QtCore.Qt.AlignCenter)
        self.Value_data_label.setObjectName("Value_data_label")
        self.gridLayout_5.addWidget(self.Value_data_label, 2, 1, 1, 1)
        self.Start_data_button = QtWidgets.QPushButton(self.frame_2)
        self.Start_data_button.setObjectName("Start_data_button")
        self.gridLayout_5.addWidget(self.Start_data_button, 0, 2, 1, 1)
        self.Glucose_data_label = QtWidgets.QLabel(self.frame_2)
        self.Glucose_data_label.setAlignment(QtCore.Qt.AlignCenter)
        self.Glucose_data_label.setObjectName("Glucose_data_label")
        self.gridLayout_5.addWidget(self.Glucose_data_label, 1, 1, 1, 1)
        self.gridLayout_5.setColumnStretch(0, 1)
        self.gridLayout_5.setColumnStretch(1, 2)
        self.gridLayout_5.setColumnStretch(2, 1)
        self.gridLayout_5.setRowStretch(0, 1)
        self.gridLayout_5.setRowStretch(1, 1)
        self.gridLayout_5.setRowStretch(2, 1)
        self.gridLayout_8.addLayout(self.gridLayout_5, 0, 1, 1, 1)
        self.verticalLayout_3.addWidget(self.frame_2)
        self.tabWidget.addTab(self.tab_5, "")
        self.gridLayout_2.addWidget(self.tabWidget, 0, 0, 1, 1)
        self.gridLayout_2.setRowStretch(0, 8)
        self.gridLayout_3.addLayout(self.gridLayout_2, 0, 0, 1, 1)
        self.gridLayout.addWidget(self.frame, 0, 0, 1, 1)
        ClinicianWindow.setCentralWidget(self.centralwidget)

        # Plot settings
            # Add grid
        self.graphWidget_CV.showGrid(x=True, y=True)
        self.graphWidget_AMP.showGrid(x=True, y=True)
            # Set background color
        self.graphWidget_CV.setBackground('w')
        self.graphWidget_AMP.setBackground('w')
            # Add title
        self.graphWidget_CV.setTitle("CV scan")
        self.graphWidget_AMP.setTitle("Amperometry scan")
            # Add axis labels
        styles = {'color':'k', 'font-size':'15px'}
        self.graphWidget_CV.setLabel('left', 'Current [uA]', **styles)
        self.graphWidget_CV.setLabel('bottom', 'Potential [V]', **styles)

        self.graphWidget_AMP.setLabel('left', 'Current [uA]', **styles)
        self.graphWidget_AMP.setLabel('bottom', 'Time [s]', **styles)
            # Add legend
        self.graphWidget_CV.addLegend()
        self.graphWidget_AMP.addLegend()


        # Connect update buttons
        self.Scan_update_button.clicked.connect(lambda state, x=UPDATE_SCAN: self.update_values_CV(state, x))
        self.Initial_update_button.clicked.connect(lambda state, x=UPDATE_INITIAL: self.update_values_CV(state, x))
        self.Final_update_button.clicked.connect(lambda state, x=UPDATE_FINAL: self.update_values_CV(state, x))
        self.Time_update_button.clicked.connect(lambda state, x=UPDATE_TIME: self.update_values_CV(state, x))

        self.Initial_update_button.setDisabled(False)
        self.Final_update_button.setDisabled(False)           
        self.Scan_update_button.setDisabled(False)            
        self.Time_update_button.setDisabled(False)


        self.retranslateUi(ClinicianWindow)
        self.tabWidget.setCurrentIndex(0)
        QtCore.QMetaObject.connectSlotsByName(ClinicianWindow)
        ClinicianWindow.setTabOrder(self.Connection_button, self.Connection_label)
        ClinicianWindow.setTabOrder(self.Connection_label, self.Connection_port_label)
        ClinicianWindow.setTabOrder(self.Connection_port_label, self.Final_update_button)
        ClinicianWindow.setTabOrder(self.Final_update_button, self.Clear_CV_button)
        ClinicianWindow.setTabOrder(self.Clear_CV_button, self.Initial_CV_textEdit)
        ClinicianWindow.setTabOrder(self.Initial_CV_textEdit, self.Initial_update_button)
        ClinicianWindow.setTabOrder(self.Initial_update_button, self.Final_CV_textEdit)
        ClinicianWindow.setTabOrder(self.Final_CV_textEdit, self.tabWidget)
        ClinicianWindow.setTabOrder(self.tabWidget, self.Scan_CV_textEdit)
        ClinicianWindow.setTabOrder(self.Scan_CV_textEdit, self.Scan_update_button)
        ClinicianWindow.setTabOrder(self.Scan_update_button, self.graphWidget_AMP)
        ClinicianWindow.setTabOrder(self.graphWidget_AMP, self.Start_amp_button)
        ClinicianWindow.setTabOrder(self.Start_amp_button, self.Draw_CV_button)



    def retranslateUi(self, ClinicianWindow):
        _translate = QtCore.QCoreApplication.translate
        ClinicianWindow.setWindowTitle(_translate("ClinicianWindow", "MainWindow"))
        self.Connection_button.setText(_translate("ClinicianWindow", ""))
        self.Connection_label.setText(_translate("ClinicianWindow", "NOT CONNECTED"))
        self.Time_CV_label.setText(_translate("ClinicianWindow", "TIME DURATION (sec):"))
        self.Initial_value_CV_label.setText(_translate("ClinicianWindow", "SET INITIAL VALUE (mV):"))
        self.Final_value_CV_label.setText(_translate("ClinicianWindow", "SET FINAL VALUE (mV):"))
        self.Scan_update_button.setText(_translate("ClinicianWindow", "UPDATE"))
        self.Initial_update_button.setText(_translate("ClinicianWindow", "UPDATE"))
        self.Scanrate_CV_label.setText(_translate("ClinicianWindow", "SET SCAN RATE (mV/sec):"))
        self.Final_update_button.setText(_translate("ClinicianWindow", "UPDATE"))
        self.Time_update_button.setText(_translate("ClinicianWindow", "UPDATE"))
        self.Draw_CV_button.setText(_translate("ClinicianWindow", "DRAW"))
        self.Clear_CV_button.setText(_translate("ClinicianWindow", "CLEAR"))
        self.Ready_CV_label.setText(_translate("ClinicianWindow", "NOT READY"))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab_3), _translate("ClinicianWindow", "Cyclic Voltammetry"))
        self.Start_amp_button.setText(_translate("ClinicianWindow", "START"))
        self.Stop_amp_button.setText(_translate("ClinicianWindow", "STOP"))
        self.Fetch_amp_button.setText(_translate("ClinicianWindow", "FETCH FROM MEMORY"))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab_4), _translate("ClinicianWindow", "Chronoamperometry"))
        self.Value_data_label.setText(_translate("ClinicianWindow", "VALUE"))
        self.Start_data_button.setText(_translate("ClinicianWindow", "START MEASUREMENT"))
        self.Glucose_data_label.setText(_translate("ClinicianWindow", "GLUCOSE CONCENTRATION (mg/dL)"))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab_5), _translate("ClinicianWindow", "Data Visualization"))
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
            i=i+1



    def update_plot_data(self, potential_CV, current_CV):


        self.x.append(potential_CV)  # Add a new value 1 higher than the last.

        self.y.append(current_CV)  # Add a new random value.

        self.CV_line.setData(self.x, self.y)  # Update the data.


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


    @pyqtSlot(bool)
    def on_toggle(self, checked):
        """!
        @brief Allow connection and disconnection from selected serial port.
        """
        if checked:

            
            self.Initial_update_button.setDisabled(True)
            self.Final_update_button.setDisabled(True)           
            self.Scan_update_button.setDisabled(True)            
            self.Time_update_button.setDisabled(True)

            # kill thread
            self.Connection_label.setStyleSheet("background-color:rgb(255,0,0);")
            self.Connection_label.setText("NOT CONNECTED")
            self.serial_worker.is_killed = True
            self.serial_worker.killed()
            self.Connection_port_label.setDisabled(False) 
            self.Connection_button.setText(
                "Connect to port {}".format(self.port_text)
            )
            self.CHECK_TOGGLE = bool(False)


        else:
            self.Initial_update_button.setDisabled(False)
            self.Final_update_button.setDisabled(False)           
            self.Scan_update_button.setDisabled(False)            
            self.Time_update_button.setDisabled(False)

            self.connect_to_COM()

            self.CHECK_TOGGLE = bool(True)

    @pyqtSlot()
    def update_values_CV(self, state, char):
        """!
        @brief Handle the ON/OFF status of the PSoC LED.

        @param state is the state of the button
        @param char is the char to be sent on serial port
        """
        global final_value
        global initial_value
        global time_value
        global scan_rate

        global UPDATE_SCAN
        global UPDATE_INITIAL
        global UPDATE_FINAL
        global UPDATE_TIME
        global UPDATE_TIME_FLAG
        global UPDATE_SCAN_FLAG


        self.serial_worker.send(char)
        time.sleep(0.1)

        if (char == UPDATE_SCAN):
            logging.info("Received: {}".format(self.Scan_CV_textEdit.toPlainText()))
            scan_rate = int(self.Scan_CV_textEdit.toPlainText())

            b=int(scan_rate>>8)
            c=int(scan_rate & (0xFF))
            self.serial_worker.port.write([b])
            time.sleep(0.1)
            self.serial_worker.port.write([c])
            time.sleep(0.1)  

            UPDATE_SCAN_FLAG=True

        elif (char == UPDATE_INITIAL):
            
            logging.info("Received: {}".format(self.Initial_CV_textEdit.toPlainText()))
            initial_value = int(self.Initial_CV_textEdit.toPlainText())

            b=int(initial_value>>8)
            c=int(initial_value & (0xFF))
            self.serial_worker.port.write([b])
            time.sleep(0.1)
            self.serial_worker.port.write([c])
            time.sleep(0.1)     

        elif (char == UPDATE_FINAL):
            logging.info("Received: {}".format(self.Final_CV_textEdit.toPlainText()))

            final_value = int(self.Final_CV_textEdit.toPlainText())
           
            b=int(final_value>>8)
            c=int(final_value & (0xFF))
            self.serial_worker.port.write([b])
            time.sleep(0.1)
            self.serial_worker.port.write([c])
            time.sleep(0.1)     

        elif (char == UPDATE_TIME):
            logging.info("Received: {}".format(self.Time_CV_textEdit.toPlainText()))

            time_value = int(self.Time_CV_textEdit.toPlainText())
            self.serial_worker.port.write([time_value])          
            UPDATE_TIME_FLAG=True

        self.serial_worker.send('z')
        time.sleep(0.1)
        
        if (self.serial_worker.read() == "CV ready"):
            
            if UPDATE_SCAN_FLAG:
                
                time_value = int(((final_value - initial_value)/scan_rate)*2)
                time_str = str(time_value)

                logging.info("Final_value: {}".format(final_value))
                logging.info("Initial_value: {}".format(initial_value))
                logging.info("Scan_rate: {}".format(scan_rate))

                self.Time_CV_textEdit.setPlainText(time_str)

            elif UPDATE_TIME_FLAG:
                scan_rate = int((final_value - initial_value)/time_value)
                scan_rate_str = str(scan_rate)
                self.Scan_CV_textEdit.setPlainText(scan_rate_str)


            self.Draw_CV_button.setDisabled(False)
            self.Ready_CV_label.setStyleSheet("background-color:rgb(0,255,0);")
            self.Ready_CV_label.setText("READY")  
            time.sleep(0.1)


    def check_serialport_status(self, port_name, status):
        """!
        @brief Handle the status of the serial port connection.

        Available status:
            - 0  --> Error during opening of serial port
            - 1  --> Serial port opened correctly
        """
        if status == 0:
            self.Connection_button.setChecked(False)
        elif status == 1:
            # enable all the widgets on the interface
            self.Connection_button.setText(
                "Disconnect from port {}".format(port_name)

            )

            self.Connection_label.setStyleSheet("background-color:rgb(0,255,0);")
            self.Connection_label.setText("CONNECTED")    


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

    ##################
    # GRAPH METHODS #
    ##################
    @pyqtSlot()
    def draw_CV(self):
        """!
        @brief Draw the plots.
        """
        global RECEIVE_CV_DATA

        self.serial_worker.send('F')
        time.sleep(0.1)
        self.serial_worker.send('z')
        time.sleep(0.1)
 
        self.CV_line = self.plot(self.graphWidget_CV, self.x, self.y, 'CV','r')

        self.serial_worker.is_killed = True
        self.serial_worker.killed()
        time.sleep(1)

        self.graph_worker = UpdateGraphWorker(port_name_global) # needs to be re defined
        # connect worker signals to functions
        self.graph_worker.graph_signals.plot_values.connect(self.update_plot_data)
 
        #self.graph_worker.graph_signals.data_ready_signal.connect(self.update_plot_data)

        # execute the worker
        self.threadpool.start(self.graph_worker)

        RECEIVE_CV_DATA  = True












    @pyqtSlot()
    def draw_AMP(self):
        """!
        @brief Draw the plots.
        """
        self.AMP_line = self.plot(self.graphWidget_AMP, self.x, self.y, 'AMP','r')


    @pyqtSlot()
    def plot(self, graph, x, y, curve_name, color):  #method called by the draw method
        """!
        @brief Draw graph.
        """
        pen = pg.mkPen(color=color)
        line = graph.plot(x, y, name=curve_name, pen=pen)
        return line

#############
#  RUN APP  #
#############


if __name__ == "__main__":
    import sys
    app = QtWidgets.QApplication(sys.argv)
    ClinicianWindow = QtWidgets.QMainWindow()
    ui = Ui_ClinicianWindow()
    ui.setupUi(ClinicianWindow)
    ClinicianWindow.show()
    sys.exit(app.exec_())
