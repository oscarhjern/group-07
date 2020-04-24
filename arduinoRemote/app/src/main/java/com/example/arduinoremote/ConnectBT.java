package com.example.arduinoremote;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import java.io.OutputStream;
import android.os.AsyncTask;
import android.util.Log;

import java.io.IOException;
import java.util.Set;
import java.util.UUID;

public class ConnectBT extends AsyncTask<Void, Void, Void> {
    String carName = "Smartcar"; //this should be the arduino cars bt address
    BluetoothAdapter myBluetooth = null;
    BluetoothSocket btSocket = null;
    BluetoothDevice device = null;
    OutputStream btOutputStream;


    @Override
    protected Void doInBackground(Void... devices) { //while the progress dialog is shown, the connection is done in background
        try {
            Log.d("Testing","Background");

                myBluetooth = BluetoothAdapter.getDefaultAdapter();//get the mobile bluetooth device

                Set<BluetoothDevice> pairedDevices = myBluetooth.getBondedDevices();

                if(pairedDevices.size() > 0) {

                    for(BluetoothDevice device: pairedDevices) {

                        if(device.getName().equals(carName)) {
                            this.device = device;
                            UUID uuid = UUID.fromString("00001101-0000-1000-8000-00805f9b34fb"); //Standard SerialPortService ID
                            btSocket = device.createRfcommSocketToServiceRecord(uuid);
                            btSocket.connect();
                            btOutputStream = btSocket.getOutputStream();
                            break;
                        }
                    }
                }
        }
        catch (IOException ignored) {
        }
        return null;
    }
}