from pymongo import MongoClient
from bson.objectid import ObjectId

class AnimalShelter(object):
    """ CRUD operations for Animal collection in MongoDB """

    def __init__(self, username, password):
        # Initializing the MongoClient. This helps to
        # access the MongoDB databases and collections.
        # This is hard-wired to use the aac database, the
        # animals collection, and the aac user.
        # Definitions of the connection string variables are
        # unique to the individual Apporto environment.
        #
        # You must edit the connection variables below to reflect
        # your own instance of MongoDB!
        #
        # Connection Variables
        #
        USER = 'aacuser'
        PASS = 'SNHU1234'
        HOST = 'nv-desktop-services.apporto.com'
        PORT = 30462
        DB = 'AAC'
        COL = 'animals'
        #
        # Initialize Connection
        #
        self.client = MongoClient('mongodb://%s:%s@%s:%d' % (USER,PASS,HOST,PORT))
        self.database = self.client['%s' % (DB)]
        self.collection = self.database['%s' % (COL)]

# Complete this create method to implement the C in CRUD.
    def create(self, data):
        if data is not None:
            createTrue = self.database.animals.insert_one(data)  # data should be dictionary
            if createTrue != 0:
                return True
            return False
        else:
            raise Exception("Nothing to save, because data parameter is empty")

# Create method to implement the R in CRUD.
    def read(self, read=None):
        if read is not None:
            data = self.database.animals.find(read, {"_id": False})
            for document in data:
                print(document)
        else:
            data = self.database.animals.find({}, {"_id": False})
        return data
            
# Create a method to implement the U in CRUD
    def update(self, initial, update):
        if initial is not None:
            if self.database.animals.count_documents(initial, limit = 1) != 0:
                update_result = self.database.animals.update_many(initial, {"$set": update})
                result = update_result.raw_result
            else:
                result = "No document found"
            return result
        else:
            raise Exception("Nothing to  update, because data parameter is empty")

# Create a method to implement the D in CRUD
    def delete(self, delete):
        if delete is not None:
            if self.database.animals.count_documents(delete, limit = 1) != 0:
                delete_result = self.database.animals.delete_many(delete)
                result = delete_result.raw_result
            else:
                result = "No document found"
            return result
        else:
            raise Exception("Nothing to delete, because data parameter is empty")
