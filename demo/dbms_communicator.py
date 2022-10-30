import socket
import time
import select

def read_int(socket, size):
	data = socket.recv(size)
	i = int.from_bytes(data, "little")
	return i

def read_string(socket, size):
	data = socket.recv(size)
	data = data.split(b'\x00')[0]

	#print(data)

	string = ""
	string = data.decode()
	#string = (data.split("\x00")[0]).decode('utf-8', errors='replace')
	

	return string



def read_int_array(socket, size, count):
	e_size = int(size / count)
	ints = []

	for i in range(count):
		ints.append(read_int(socket, e_size))

	return ints



def read_string_array(socket, size, count):
	e_size = int(size / count)
	strings = []

	for i in range(count):
		strings.append(read_string(socket, e_size))

	return strings


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
		data = []

		ready = select.select([self.sock], [], [], 0.5)
		if ready[0]:
			columns = read_int(self.sock, 8)
			length = read_int(self.sock, 8)

			for i in range(columns):
				dtype = read_int(self.sock, 8)
				dsize = read_int(self.sock, 8)
				if (dtype == 1):
					col_data = read_string_array(self.sock, dsize, length)
					data.append(col_data)
				else:
					col_data = read_int_array(self.sock, dsize, length)
					data.append(col_data)

		print(data)
		return data

	def close_connection(self):
		self.sock.close()
		self.connected = False

