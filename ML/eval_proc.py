#!/usr/bin/env python3

import socket
import torch
import sys
import torch.nn as nn
import numpy as np 

PATH= "/home/sgundo/Desktop/Tesi/OS_FaultMonitor/ML/"
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

def eval(feat):
		X = np.array(feat)
		X = torch.from_numpy(X).float()
		X = X.unsqueeze(0)
		y = model(X)
		y = torch.log_softmax(y, dim=1)
		_, y = torch.max(y, dim = 1)   
		
		return y.item()

f_s = True # read metrics from file or receive it from socket
flip = 0
cmd = sys.argv[1]
bi = sys.argv[2]
model = BinaryClassification()
model.load_state_dict(torch.load(PATH+bi+".pth", map_location=torch.device('cpu')))
model.eval()
HOST = '127.0.0.1'  # Standard loopback interface address (localhost)
PORT = 2325        # Port to listen on (non-privileged ports are > 1023)
i = 0
num_feat = 7

if f_s:
	with open(cmd+".out", "r") as file:
		first_line = file.readline()
		for last_line in file:
			pass
		print (last_line)
		attr = last_line.split()
		attr = attr[3:]
		print(attr)
		attr_int = [int(i) for i in attr]
		if len(attr_int) == 7:
			eval(attr_int)

else:
	with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
		s.bind((HOST, PORT))
		s.listen()
		conn, addr = s.accept()
		with conn:
			print('Connected by', addr)
			while True:
				i = 0
				feat = list()
				for i in range(0, num_feat+1):
					data = conn.recv(4)
					num = int.from_bytes(data, byteorder='little', signed=False)
					if i==0:
						bina=num
						continue
					feat.append(num)
				if not data:
					break
				print("Evaluate process bin: "+str(bina))
				result = eval(feat)
				if flip == 0:
					result=1
					flip=1
				else:
					flip=0
					result=0
				if result == 0:
					conn.sendall(b"0")
					print("Sent not fault valuation")
				else:
					conn.sendall(b"1")
					print("Sent fault valuation")
