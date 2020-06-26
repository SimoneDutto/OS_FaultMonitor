#!/usr/bin/env python3

import socket
import torch
import torch.nn as nn

PATH= "model_NN50_20.pth"
len_index = 20

class BinaryClassification(nn.Module):
    def __init__(self):
        super(BinaryClassification, self).__init__()

        self.features = nn.Sequential(
              nn.Linear(len_index-1, 32),
              nn.BatchNorm1d(32),
              nn.Dropout(0.5), #50 % probability 
              nn.LeakyReLU(),
              torch.nn.Linear(32, 64),
              nn.BatchNorm1d(64),
              torch.nn.Dropout(0.2), #20% probability
              torch.nn.LeakyReLU(),
              torch.nn.Linear(64, 22),
              nn.BatchNorm1d(22),
              torch.nn.LeakyReLU())
        self.classifier = torch.nn.Linear(22, 2)
        
    def forward(self, x):
        x = self.features(x)
        x = self.classifier(x)
        return x

def eval():
        y = model(X)
        y = torch.log_softmax(y, dim=1)
        _, y = torch.max(y, dim = 1)   
        print("Result: "+str(y))
    	
        return y


model = BinaryClassification()
model.load_state_dict(torch.load(PATH, map_location=torch.device('cpu')))
model.eval()

HOST = '127.0.0.1'  # Standard loopback interface address (localhost)
PORT = 2325        # Port to listen on (non-privileged ports are > 1023)
i = 0
num_feat = 1
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    conn, addr = s.accept()
    with conn:
        print('Connected by', addr)
        while True:
            i = 0
            feat = list()
            for i in range(0, num_feat):
            	
            	data = conn.recv(4)
            	
            	num = int(data)
            	print (num)
            	feat.append(num)
            
            
            print (num)
            if not data:
                break
                
            result = eval();
            if i == 0:
            	conn.sendall(b+str(result))
            	i = 1
            else:
            	conn.sendall(b+str(result))
            	i = 0
