import torch
from model import get_model, get_transform
from dataset import CupDataset
from torch.utils.data import DataLoader

device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

num_classes = 2
model = get_model(num_classes)
model.load_state_dict("fasterrcnn_cup_model.pth")
model.to(device)
model.eval()

someimagefile = 0 # get this from opencv

with torch.no_grad():
    img = someimagefile
    prediction = model([img.to(device)])
    print(prediction)
