import socket
import time
import select

def read_string(socket, size):
	data = socket.recv(size)
	i = int.from_bytes(data, "little")
	return i

def read_int(socket, size):
	data = socket.recv(size)
	string = data.decode('utf-8')
	return string

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


	def fetch_result(self):
		print("whaat")

		data = None
		ready = select.select([self.sock], [], [], 0.5)
		if ready[0]:
			data = self.sock.recv(8)

		if not (data is None):
			size = int.from_bytes(data, "little")
			print(size)

			e_size = int(size / 5);

			integer_list = []
			for i in range(5):
				string = read_string(self.sock, e_size)
				print(string)
				#integer = int.from_bytes(byte_arr, "little")
				#integer_list.append(integer)

			#print(integer_list)



	def close_connection(self):
		self.sock.close()
		self.connected = False

