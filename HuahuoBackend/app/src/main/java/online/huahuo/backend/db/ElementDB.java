package online.huahuo.backend.db;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import javax.persistence.*;

@Entity
@Data
@NoArgsConstructor
@AllArgsConstructor
@Table(name = "ELEMENTS")
public class ElementDB {
    @Id
    @Column(name = "id", nullable = false)
    private String elementId;

    private boolean editable; // Allow other people to edit this element.
    private boolean shareable; // Allow other people to share this element.

    @OneToOne()
    @JoinColumn(name = "fileId", referencedColumnName = "id")
    private BinaryFileDB binaryFileDB;
}
