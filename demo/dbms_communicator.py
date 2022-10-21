import socket
import time

class DBMS_COM(object):

	"""communicator for the Database management system"""
	def __init__(self, host, port):
		self.host = host
		self.port = port
		self.connected = False

		self.connect_to_host()


	def connect_to_host(self):
		if not self.connected:
			self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
			self.sock.connect((self.host, self.port))
			#message = self.sock.recv(64)
			self.connected = True
			#print(message)


	def send_query(self, query):
		self.sock.sendall(bytearray(query, "utf-8"))

	def close_connection(self):
		self.sock.close()
		self.connected = False

