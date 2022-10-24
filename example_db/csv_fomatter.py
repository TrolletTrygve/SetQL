import csv
import collections

FILE 		= "cities/worldcities.csv"
OUTPUT1 	= "cities/step1.csv"
OUTPUT2 	= "cities/step2.csv"
OUTPUT3 	= "cities/step3.csv"

LANGUAGE_OUTPUT = "language_example.txt"

cities_ascii 	= []
country 		= []
capital_status 	= []
population 		= []


def round_down_population(groups, pop):
	for i in range(len(groups)):
		if i + 1 == len(groups):
			return groups[i]

		if pop >= groups[i] and pop < groups[i+1]:
			return groups[i]

	return -1


# Read the initial CSV file
with open(FILE, encoding="utf8") as file:
	reader = csv.reader(file, delimiter=';')
	header = next(reader)

	for row in reader:

		pop = None
		try:
			pop = int(row[9])
		except:
			print("skipping", row[1])
			continue

		cities_ascii.append(row[1])
		country.append(row[4])

		cap_status = row[8]
		if (cap_status == ""):
			cap_status = "none"

		capital_status.append(cap_status)

		population.append(pop)

		print(row[1])


# Write a CSV only containing the relevant columns
with open(OUTPUT1, 'w', encoding="utf8", newline='') as file:
	writer = csv.writer(file)

	for i in range(len(cities_ascii)):
		row = [cities_ascii[i], country[i], capital_status[i], population[i]]
		writer.writerow(row)



population_groups = [
	0,
	1000,
	50000,
	100000,
	500000,
	1000000,
	2000000,
	5000000,
	10000000,
	15000000,
	20000000,
	30000000,
	35000000]

print(population_groups)


# Write a CSV only containing the relevant columns and fixed populations
with open(OUTPUT2, 'w', encoding="utf8", newline='') as file:
	writer = csv.writer(file)

	for i in range(len(cities_ascii)):

		pop = round_down_population(population_groups, population[i])

		row = [cities_ascii[i], country[i], capital_status[i], pop]
		writer.writerow(row)



num_cities			= len(set(cities_ascii))
num_countries 		= len(set(country))
num_capital_status 	= len(set(capital_status))
num_population		= len(set(population_groups))


print("city count:", num_cities)
print("country count:", num_countries)
print("capital status count:", num_capital_status)
print("population count", num_population)

dup = [item for item, count in collections.Counter(cities_ascii).items() if count > 1]
print(dup)


# Create list of country names the way they will be made into sets
real_country_names = []
for c in country:
	setName = "in" + c.replace(" ", "")
	setName = setName.replace("(", "")
	setName = setName.replace(")", "")
	real_country_names.append(setName);

real_population_names = []
for p in population:
	pop = round_down_population(population_groups, p)
	setName = "has" + str(pop)
	real_population_names.append(setName)

real_capital_status_names = []
for cs in capital_status:
	setName = "capitalStatus" + cs.capitalize()
	real_capital_status_names.append(setName)




db_sets = []
db_sets_country = []
db_sets_cs = []
db_sets_pop = []

# Add all countries as sets
for c in set(real_country_names):
	db_sets.append(c)
	db_sets_country.append(c)

for cs in set(capital_status):
	setName = "capitalStatus" + cs.capitalize()
	db_sets.append(setName)
	db_sets_cs.append(setName)

for i in range (1, len(population_groups)):
	setName = "has" + str(population_groups[i])
	db_sets.append(setName)
	db_sets_pop.append(setName)


print(db_sets)

set_countries = dict.fromkeys(db_sets_country)
for c in db_sets_country:
	new_set = []
	for i in range(len(cities_ascii)):
		if (real_country_names[i] == c):
			new_set.append(cities_ascii[i])
	set_countries[c] = new_set

set_capital_status = dict.fromkeys(db_sets_cs)
for cs in db_sets_cs:
	new_set = []
	for i in range(len(cities_ascii)):
		if (real_capital_status_names[i] == cs):
			new_set.append(cities_ascii[i])
	set_capital_status[cs] = new_set

set_has_population = dict.fromkeys(db_sets_pop)
for p in population_groups:
	new_set = []
	for i in range(len(cities_ascii)):
		if (population[i] >= p):
			new_set.append(cities_ascii[i])
	i = population_groups.index(p) - 1
	if i < 0: 
		continue
	set_has_population[db_sets_pop[i]] = new_set




with open(LANGUAGE_OUTPUT, "w") as file:
	# define stuff
	file.write("CREATE UNIVERSE Cities(name STRING);\n")
	file.write("CREATE ATTRIBUTES Cities(population INTEGER);\n")

	for s in db_sets:
		row = "CREATE SET " +  s + ";\n"
		file.write(row)

	for i in range(len(cities_ascii)):
		row = 'INSERT {"' + cities_ascii[i] + '":(' + str(population[i]) + ')} INTO Cities;\n'
		file.write(row)

	for key in set_countries:
		for c in set_countries[key]:
			row = 'INSERT {"' + c + '"} INTO ' + key + ';\n'
			file.write(row)

	for key in set_capital_status:
		for cs in set_capital_status[key]:
			row = 'INSERT {"' + cs + '"} INTO ' + key + ';\n'
			file.write(row)

	for key in set_has_population:
		if (set_has_population[key] is None):
			continue
		for p in set_has_population[key]:
			row = 'INSERT {"' + p + '"} INTO ' + key + ';\n'
			file.write(row)


print(real_population_names)
print(real_capital_status_names)

print(set_countries)
print(set_capital_status)

print(set_has_population)