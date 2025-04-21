import torch
from torch.utils.data import DataLoader
import torchvision.transforms as T
from model import get_model
from dataset import CupDataset

def get_transform():
    return T.Compose([
        T.ToTensor(),
    ])
    
dataset = CupDataset("your_dataset", transforms=get_transform())
data_loader = DataLoader(dataset, batch_size=2, shuffle=True, collate_fn=lambda x: tuple(zip(*x)))

device = torch.device("cuda") if torch.cuda.is_available() else torch.device("cpu")
model = get_model(num_classes=2)
model.to(device)

# Optimizer
params = [p for p in model.parameters() if p.requires_grad]
optimizer = torch.optim.SGD(params, lr=0.005, momentum=0.9, weight_decay=0.0005)

# Training loop
num_epochs = 10
model.train()
for epoch in range(num_epochs):
    for images, targets in data_loader:
        images = list(img.to(device) for img in images)
        targets = [{k: v.to(device) for k, v in t.items()} for t in targets]

        loss_dict = model(images, targets)
        losses = sum(loss for loss in loss_dict.values())

        optimizer.zero_grad()
        losses.backward()
        optimizer.step()

    print(f"Epoch {epoch+1}, Loss: {losses.item():.4f}")
torch.save(model.state_dict(), "fasterrcnn_cup_model.pth")
