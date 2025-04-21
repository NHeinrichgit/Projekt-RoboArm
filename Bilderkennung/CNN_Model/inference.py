import torch
from model import get_model, get_transform
from dataset import CupDataset

device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

num_classes = 2
model = get_model(num_classes)
model.load_state_dict("fasterrcnn_cup_model.pth")
model.to(device)
model.eval()

dataset = CupDataset(".", transforms= get_transform())

with torch.no_grad():
    img, _ = dataset[0]
    prediction = model([img.to(device)])
    print(prediction)
