import torch
from model import get_model
import torchvision.transforms as T

transform = T.Compose([
    T.ToTensor(),
])

def initmodel():
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

    num_classes = 4
    model = get_model(num_classes)
    state_dict = torch.load("fasterrcnn_cup_model.pth", map_location=device)
    model.load_state_dict(state_dict)
    model.to(device)
    model.eval()
    return model

def checkimg(model, imagefile):
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    with torch.no_grad():
        imagetensor = transform(imagefile)
        prediction = model([imagetensor.to(device)])
        return prediction
